 ; Contains functions to access the Floppy.
 ;
 ; Copyright (c) 2012, Shikhin Sethi
 ; All rights reserved.
 ;
 ; Redistribution and use in source and binary forms, with or without
 ; modification, are permitted provided that the following conditions are met:
 ;     * Redistributions of source code must retain the above copyright
 ;       notice, this list of conditions and the following disclaimer.
 ;     * Redistributions in binary form must reproduce the above copyright
 ;       notice, this list of conditions and the following disclaimer in the
 ;       documentation and/or other materials provided with the distribution.
 ;     * Neither the name of the <organization> nor the
 ;       names of its contributors may be used to endorse or promote products
 ;       derived from this software without specific prior written permission.
 ;
 ; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 ; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 ; DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 ; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 ; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 ; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 ; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    .Size     dd 0x2000               ; The size of the BIOS File - now we know the exact number.

DBAL:
    .LBA      dd 24                   ; The LBA of the starting of the DBAL File - hardcoded, since the BIOS is packed to 8KiB.
    .Size     dd 0                    ; The size for the DBAL file is currently unknown - perhaps later it would be known.

FILE:
    .Code   db -1                     ; Code of the file opened.
    .LBA    dd 0                      ; The LBA of the sector we are going to "read next".
    .Size   dd 0                      ; The size of the file left for reading.

%define SECTORS_PER_TRACK             18
%define HEADS                         2
%define TRACKS                        80

SECTION .base

DiskError db "ERROR: Unable to access the boot floppy drive.", 0
BootError db "ERROR: Boot file corrupt.", 0

; Save the boot drive number here.
BootDrive         dd 0
Retry             db 0                ; Number of times to retry a particular function - safety purposes.

 ; Gets the complete the boot file (us).
 ;
 ; Returns:
 ;      Boot -> aborts boot if any error occured.
BootFileGet:
    ; Save some registers we are using in this function.
    push edi
    push eax
    push ecx
    push edx

    mov edi, 0x7C00 + 512             ; The load address for the sectors.
    mov al, 1                         ; Head 0; Cylinder 0; Sector 1; Read 2 sector.
    mov ch, 0
    mov cl, 2
    mov dh, 0

.LoopGet:
    call FloppyReadSector
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
    ; Restore the registers we are using in this function.
    pop edx
    pop ecx
    pop eax
    pop edi
    
    ret

.Fail1:
    mov si, DiskError
    jmp AbortBoot
 
.Fail2:
    mov si, BootError
    jmp AbortBoot

 ; Read a sector from the disk to a buffer.
 ;     EDI    -> the buffer to where to read the disk sector to.
 ;      AL    -> the number of sectors to read.
 ;      CH    -> the cylinder number.
 ;      CL    -> the sector number from 0-5 (bits).
 ;            -> the cylinder number from 6-7 (bits).
 ;      DH    -> The head number.
 ;
 ; Returns:
 ;      Carry -> set if unsuccessful.
FloppyReadSector:
    pushad
    push es
    
    ; Retry three times before failing.
    mov byte [Retry], 3
    
    ; Get the segment in ES.
    mov ebx, edi
    and edi, 0x000F
    shr ebx, 4
    mov es, bx
    
    mov bx, di
    
; It is usually recommended to try 3 times - floppy controller can have many errors.
.Loop:
    mov ah, 0x02                    
    mov dl, [BootDrive]               ; Save the boot drive number into DL.
  
    int 0x13
    jc .Error                         ; If carry is set, error occured.

.Success:
    pop es
    popad
    
    ret

.Error:
    ; If did all retry's, then abort with error.
    dec byte [Retry]
    jz .AbortWithError

    ; Or clear carry, and retry.
    clc
    jmp .Loop                         ; And Retry once again.
    
.AbortWithError:
    ; Return without clearing carry.
    pop es
    popad

    ret

SECTION .text

 ; Initializes the system so that files may be opened/closed later on (finds LBA).
BootFilesInit:
    pushad

    mov eax, [DBAL.LBA]               ; Get the LBA into EAX.
    mov ecx, 1                        ; Read one sectors.
    mov di, 0x9000                    ; We'd be reading at 0x9000 - temporary address of all these files. 
    
    call FloppyReadSectorM              ; Read from the floppy - multiple sectors, with advanced error checking.
    
    mov ecx, [0x9000 + 12]            ; Offset 12 of the file is the EOF address.
    sub ecx, 0xE000                   ; Subtract Start of File to get the size of the file.

    add ecx, 0x1FF                    ; Pad it to the last 512 byte boundary.
    and ecx, ~0x1FF
    mov [DBAL.Size], ecx              ; And store it!

.Return:
    popad
    ret

 ; Reads multiple sectors from floppy.
 ;     EAX  -> expects the LBA from where to read in EAX.
 ;     ECX  -> expects the number of sectors to read in ECX.
 ;     EDI  -> the buffer to where to read the floppy sector(s) to.
 ;
 ; Returns:
 ;     ECX  -> the number of sectors actually read.
 ;     Boot -> aborts boot if nothing made it work.
; NOTE: M stands for Multiple (sectors).
FloppyReadSectorM:
    pushad

    mov [Info.Read], ecx              ; And if we take the single path soon, put it here.

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
    mov eax, SECTORS_PER_TRACK + 1
    sub eax, [Info.Sector]            ; Subtract the sector (in the track) from SECTORS_PER_TRACK.
                                      ; That's the maximum possible sectors we can read.
  
    mov ecx, [Info.Read]              ; Get back the number of sectors to read.

    cmp ecx, eax                      ; Compare requested sectors to read, with maximum possible sectors.
    jbe .Read                         ; If below or equal, read the sectors.

    mov ecx, eax                      ; Else read maximum possible sectors.
    mov [Info.Read], ecx

; Here we read the sectors from disk - ECX should be the number of sectors we can possibly read this time.
.Read:
    cmp byte [Info.Failure], 3        ; If we have failed more than/equal to three times, do one sector at a time reads.
    jae .Single

.Multiple:
    mov al, cl                        ; How many sectors should we read?
    
    mov ch, [Info.Cylinder]           ; Since Info.Cylinder is a DWORD, and we only want the byte part, we do + 3 (from EDX).
    mov dh, [Info.Head]
    mov cl, [Info.Sector]

    call FloppyReadSector
    jnc .Return                       ; If the carry flag wasn't set, return.

.Fail:
    inc byte [Info.Failure]           ; Increase the failure byte.

.Single:
    mov ebx, [Info.Read]              ; How many sectors to read - get into EBX.

    mov al, 1                         ; We only read 1 sector - SINGLE (c'mon READ).
    mov ch, [Info.Cylinder]           ; Since Info.Cylinder is a DWORD, and we only want the byte part, we do so.
    mov cl, [Info.Sector]             ; ABOVE.
    mov dh, [Info.Head]               ; ABOVE.
    
; The single loop, reading EBX times.
.LoopSingle:
    call FloppyReadSector
    jc .Error

    ; Decrease the count of sectors to read.
    dec ebx 
    ; If read all sectors, end.
    jz .Return
    
    inc cl                            ; Increase the sector (which we are reading).
    add edi, 0x200                    ; Add 512 to the destination buffer.
    jmp .LoopSingle

.Return:
    popad
    mov ecx, [Info.Read]
    ret

.Error:
    mov si, DiskError
    jmp AbortBoot

 ; Opens a file to be read from.
 ;      AL   -> contains the code number of the file to FILE.
 ;       0   -> common BIOS File.
 ;       1   -> DBAL.
 ;       2   -> background image.
 ;
 ; Returns: 
 ;     Carry -> set if any error occured.
 ;     ECX   -> the size of the file you want to FILE.
FileOpen:
    push eax
    
    ; If file code isn't -1, then return with error.
    cmp byte [FILE.Code], -1
    jne .Error

    cmp al, 0
    je .BIOS                          ; Code 0 indicates the common BIOS file.

    cmp al, 1                         ; While 1 indicates the DBAL file.
    je .DBAL

    ; Don't recognize Background image in floppies.
    jmp .Error

.BIOS:
    mov [FILE.Code], al

    mov eax, [BIOS.LBA]               ; Get the LBA in EAX.
    mov [FILE.LBA], eax

    mov eax, [BIOS.Size]              ; Get the Size in EAX.
    mov [FILE.Size], eax

    jmp .Return

.DBAL:
    mov [FILE.Code], al

    mov eax, [DBAL.LBA]               ; Get the LBA in EAX.
    mov [FILE.LBA], eax

    mov eax, [DBAL.Size]
    mov [FILE.Size], eax              ; And the size.

.Return:
    mov ecx, [FILE.Size] 
    
    pop eax
    ret

.Error:
    ; Set the file code to 0, and the carry flag.
    mov byte [FILE.Code], 0
    stc 
    
    pop eax
    ret

 ; Reads the 'next LBA' of the file currently opened.
 ;     EDI  -> the destination address of where to read the file to.
 ;     ECX  -> the number of bytes to read.
 ;
 ; Returns:
 ;     Boot -> aborted if any error occured.
FileRead:
    pushad
    
    add ecx, 0x1FF
    and ecx, ~0x1FF                   ; Pad it to the nearest 1FF byte boundary (512).
    cmp ecx, [FILE.Size]              ; If size we want to read <= size we can read continue;
    
    jbe .Cont
  
    mov ecx, [FILE.Size]              ; Else, we read only [FILE.Size] bytes.

.Cont:
    sub [FILE.Size], ecx              ; Subtract bytes read from bytes we can read.

.Read:
    mov eax, [FILE.LBA]               ; Get the LBA to read in EBX.
    add ecx, 0x1FF
    shr ecx, 9                        ; And the number of sectors to read in ECX.
    
    mov edx, ecx                      ; Keep that for internal count.

; Here we have the number of sectors to read in ECX, the LBA in EAX and the destination buffer in EDI. Let's shoot!
.Loop:
    call FloppyReadSectorM              ; Do the CALL!
   
    add eax, ecx                      ; Advance the LBA by read sectors count.

    sub edx, ecx                      ; EDX more sectors left to do.
    jz .Return                        ; Read all sectors, return.
   
    ; Now need to advance EDI.
    mov ebp, ecx
    shl ebp, 9
    add edi, ebp
    
    mov ecx, edx                      ; If not, read EDX (sectors left to do) sectors next time.
    jmp .Loop
    
.Return:
    mov [FILE.LBA], eax               ; Store the next LBA in FILE.LBA

    popad
    ret

 ; Closes the file currently opened.
FileClose:
    mov byte [FILE.Code], -1
    ret