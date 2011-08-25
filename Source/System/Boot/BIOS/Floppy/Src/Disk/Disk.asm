; Contains functions to access the Floppy.
;
; Copyright (c) 2011 Shikhin Sethi
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


SECTION .base

; This section contains all the macros.
%define           nl 0x0A, 0x0D
DiskError db "ERROR: Unable to access the boot floppy drive.", nl, 0
BootError db "ERROR: Boot file corrupt.", nl, 0

; Save the boot drive number here.
BootDrive         dd 0

Retry             db 0                ; Number of times to retry a particular function - safety purposes.

; Gets the complete the boot file (us).
;     @rc
;                 Aborts boot if any error occured.
GetBootFile:
    pushad

    mov edi, 0x7C00 + 512             ; The load address for the sectors.
    mov al, 1                         ; Head 0; Cylinder 0; Sector 1; Read 2 sector.
    mov ch, 0
    mov cl, 2
    mov dh, 0

.LoopGet:
    call ReadFromFloppy
    jc .Fail1
    inc cl                            ; Jump to the next sector.

    cmp cl, 8                         ; If above 4th sector, finish.
    ja .Done

    add di, 0x200
    jmp .LoopGet

.Done:
    cmp dword [0x8C00 - 8], "DRAU"    ; Compare the signature.
    jne .Fail2

    cmp dword [0x8C00 - 4], "MRSS"    ; And the rest of the signature.
    jne .Fail2

.Return:    
    popad
    ret

.Fail1:
    mov si, DiskError
    xor ax, ax
    call AbortBoot
 
.Fail2:
    mov si, BootError
    xor ax, ax
    call AbortBoot

; Read a sector from the disk to a buffer.
;     @di         The buffer to where to read the disk sector to.

;     @al         The number of sectors to read.
;     @ch         The cylinder number.
;     @cl         The sector number from 0-5 (bits).
;                 The cylinder number from 6-7 (bits).
;     @dh         The head number.
;     @rc
;                 Sets with carry if unsuccessful.
ReadFromFloppy:
    mov byte [Retry], 3
    pushad
    
    mov bx, di
; It is usually recommended to try 3 times - floppy controller can have many errors.
.Retry:
    mov ah, 0x02                    
    mov dl, [BootDrive]               ; Save the boot drive number into DL.
  
    int 0x13
    jc .Error                         ; If carry is set, error occured.

    test ah, ah
    jnz .Error                        ; If AH isn't zero, error most probably.

.Success:
    popad
    ret

.Error:
    push ecx
    
    xor ecx, ecx
    mov cl, [Retry]

    dec ecx                           ; Decrement ECX.
    test ecx, ecx                     ; If ECX is zero, finish retrying.
    jz .AbortWithError

    mov [Retry], cl                   ; Save the Retry value.
    
    pop ecx                           ; Restore ECX back.
    jmp .Retry                        ; And Retry once again.
    
.AbortWithError:
    pop ecx

    stc
    popad
    ret

SECTION .data

Info:
    .LBA          dd 0                ; Save the LBA here (for future use)
    .Sector       dd 0                ; Save the sector here.
    .Head         dd 0                ; Save the head here.
    .Cylinder     dd 0                ; Save the cylinder here.
    .Failure      db 0                ; How many times have we failed (reading all tracks at once).
    .Read         dd 0                ; How many sectors have we read?

BIOS:
    .LBA      dd 8                    ; The LBA for the starting of the BIOS File.
    .Size     dd 0                    ; The size of the BIOS File.

SECTION .text

%define SECTORS_PER_TRACK             18
%define HEADS                         2
%define TRACKS                        80

; Initializes the system so that files may be opened/closed later on (finds LBA).
InitBootFiles:
    pushad

    mov eax, [BIOS.LBA]               ; Get the LBA into EAX.
    mov ecx, 1                        ; Read one sectors.
    mov di, 0x9000                    ; We'd be reading at 0x9000 - temporary address of all these files. 
    
    call ReadFromFloppyM              ; Read from the floppy - multiple sectors, with advanced error checking.
    
    mov ecx, [0x9000 + 10]            ; Offset 10 of the file is the EOF address.
    sub ecx, 0x9000                   ; Subtract Start of File to get the size of the file.
    mov [BIOS.Size], ecx              ; And store it!

.Return:
    popad
    ret

; Reads multiple sectors from floppy.
; @eax            Expects the LBA from where to read in EAX.
; @ecx            Expects the number of sectors to read in ECX.
; @di             The buffer to where to read the floppy sector(s) to.

; @rc
;                 @ecx contains the number of sectors actually read.
;                 Aborts boot if nothing made it work.
; M stands for Multiple (sectors).
ReadFromFloppyM:
    pushad
    push ecx                          ; Save the number of sectors to read.

; Convert the LBA into CHS.
.Convert:
    mov [Info.LBA], eax               ; Save the LBA in Info.LBA
    xor edx, edx
    
    mov ecx, SECTORS_PER_TRACK        ; Put the sectors per track in ECX.
    div ecx                           ; Divide the LBA by sectors per track.
    
    inc edx                           ; Sector is 1 based, thus add 1 to it.
    mov [Info.Sector], edx            ; And then store it in Info.Sector.

    ; EAX will be LBA / Sectors per track - exactly what we need to calculate the future values.
    xor edx, edx                      ; Clear out EDX.

    mov ecx, HEADS
    div ecx                           ; Divide EDX:EAX by ECX.

    mov [Info.Head], edx              ; The head number if the remainder.
    mov [Info.Cylinder], eax          ; The quotient is the Cylinder - put it into Info.Cylinder.

; Checks out the maximum number of sectors we can possibly read.
.FindMax:
    mov eax, SECTORS_PER_TRACK
    sub eax, [Info.Sector]            ; Subtract the sector (in the track) from SECTORS_PER_TRACK.
                                      ; That's the maximum possible sectors we can read.

    pop ecx                           ; Get back the number of sectors to read.
    cmp ecx, eax                      ; Compare requested sectors to read, with maximum possible sectors.

    jbe .Read                         ; If below or equal, read the sectors.

    mov ecx, eax                      ; Else read maximum possible sectors.

; Here we read the sectors from disk - ECX should be the number of sectors we can possibly read this time.
.Read:
    mov [Info.Read], ecx
    cmp byte [Info.Failure], 3        ; If we have failed more than/equal to three times, do one sector at a time reads.
    jae .Single

.Multiple:
    mov al, cl                        ; How many sectors should we read?
    
    mov edx, [Info.Cylinder]
    mov ch, dl                        ; Since Info.Cylinder is a DWORD, and we only want the byte part, we do + 3 (from EDX).
    
    mov edx, [Info.Sector]
    mov cl, dl                       ; ABOVE.
    
    mov ebx, [Info.Head]
    mov dh, bl                        ; ABOVE.
    
    mov dl, [BootDrive]               ; Nothing surprising here.
    
    call ReadFromFloppy
    jnc .Return                       ; If the carry flag wasn't set, return.

    inc byte [Info.Failure]           ; Increase the failure byte.

.Single:
    mov ebx, [Info.Read]              ; How many sectors to read - get into EBX.

    mov al, 1                         ; We only read 1 sector - SINGLE (c'mon READ).
    mov ch, [Info.Cylinder + 3]       ; Since Info.Cylinder is a DWORD, and we only want the byte part, we do + 3.
    mov cl, [Info.Sector + 3]         ; ABOVE.
    mov dh, [Info.Head + 3]           ; ABOVE.
    mov dl, [BootDrive]               ; Nothing surprising here.

; The single loop, reading EBX times.
.LoopSingle:
    call ReadFromFloppy
    jc .Error

    ; Decrease the count of sectors to read.
    dec ebx
    ; If read all sectors, end.
    test ebx, ebx
    jz .Return
    
    inc cl                            ; Increase the sector (which we are reading).
    add di, 0x200                     ; Add 512 to the destination buffer.
    jmp .LoopSingle

.Return:
    popad
    mov ecx, [Info.Read]
    ret

.Error:
    mov si, DiskError
    xor ax, ax
    call AbortBoot

SECTION .data

Open:
    .IsOpen db 0                      ; Set to 1 is a file is open.
    .LBA    dd 0                      ; The LBA of the sector we are going to "read next".
    .Size   dd 0                      ; The size of the file left to read (as reported by the file system).

SECTION .text

; Opens a file to be read from.
; @al             Contains the code number of the file to open.
;                 0 -> Common BIOS File.
;     @rc 
;                 Returns with carry set if ANY error occured (technically, no error should be happening, but still).
;                 @ecx    The size of the file you want to open.
OpenFile:
    pushad
    
    mov bl, [Open]
    test bl, bl
    jnz .Error

    mov byte [Open], 1

    cmp al, 0
    jne .Error                        ; Currently you can only open the Common BIOS FILE!
   
.BIOS:
    mov eax, [BIOS.LBA]               ; Get the LBA in EAX.
    mov [Open.LBA], eax

    mov eax, [BIOS.Size]              ; Get the Size in EAX.
    mov [Open.Size], eax

.Return:
    popad
    mov ecx, [Open.Size] 
    ret

.Error:
    stc 
    popad
    ret

; Reads the 'next LBA' of the file currently opened.
; @edi            The destination address of where to read the file to.
; @ecx            The number of bytes to read.
;     @rc
;                 Aborts boot if any error occured (during read, that is).
ReadFile:
    pushad
    cmp ecx, [Open.Size]              ; If size we want to read <= size we can read continue;

    jbe .Cont
  
    mov ecx, [Open.Size]              ; Else, we read only [Open.Size] bytes.

.Cont:
    sub [Open.Size], ecx              ; Subtract bytes read from bytes we can read.

.Read:
    mov eax, [Open.LBA]               ; Get the LBA to read in EBX.
    add ecx, 0x1FF
    shr ecx, 9                        ; And the number of sectors to read in ECX.

    mov edx, ecx                      ; Keep that for internal count.

; Here we have the number of sectors to read in ECX, the LBA in EAX and the destination buffer in EDI. Let's shoot!
.Loop:
    call ReadFromFloppyM              ; Do the CALL!

    sub edx, ecx                      ; EDX more sectors left to do.
    test edx, edx
    jz .Return                        ; Read all sectors, return.

    add eax, ecx                      ; Advance the LBA by read sectors count.
   
    ; Now need to advance EDI.
    push eax                          ; Save EAX - and restore it later.

    mov eax, ecx                      ; Get the sectors read count in ECX.
    mov ebx, 512
    mul ebx                           ; And multiply it by 512, and advance EDI by it.

    add edi, eax

    pop eax
    
    mov ecx, edx                      ; If not, read EDX (sectors left to do) sectors next time.
    jmp .Loop

.Return:
    popad
    ret

; Closes the file currently opened.
CloseFile:
    mov byte [Open], 0
    ret
