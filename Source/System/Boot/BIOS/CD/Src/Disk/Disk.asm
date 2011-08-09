; Contains functions to access the Disk.
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


SECTION .text

; Provide a table for disk error messages such that printing them becomes easier.
DiskErrTable1:
    dw 0
    dw DiskErrCmd01
    dw DiskErrCmd02
    dw DiskErrCmd03
    dw DiskErrCmd04
    dw 0
    dw 0
    dw 0
    dw DiskErrCmd08
    dw DiskErrCmd09
    dw 0
    dw 0
    dw DiskErrCmd0C
    dw 0
    dw 0
    dw 0
    dw DiskErrCmd10

; Provide a second table, so that each table takes less space.
DiskErrTable2:
    dw DiskErrCmdB0
    dw DiskErrCmdB1
    dw DiskErrCmdB2
    dw DiskErrCmdB3
    dw DiskErrCmdB4
    dw DiskErrCmdB5
    dw DiskErrCmdB6


; This section contains all the macros.
%define           nl 0x0A, 0x0D


; This section contains all the Strings.
DiskErr0          db "ERROR: Read from CD failed.", nl, 0
DiskErr1          db "ERROR: Not able to find the Primary Volume Descriptor.", nl, 0
DiskErr2          db "ERROR: Int 0x13 extension not supported.", nl, 0

; Error messages returned by a BIOS in AH during disk operations.
DiskErrCmd01      db "ERROR: Invalid function in AH or invalid parameter.", nl, 0
DiskErrCmd02      db "ERROR: Address mark not found.", nl, 0
DiskErrCmd03      db "ERROR: Disk write-protected.", nl, 0
DiskErrCmd04      db "ERROR: Sector not found/read error.", nl, 0
DiskErrCmd08      db "ERROR: DMA overrun.", nl, 0
DiskErrCmd09      db "ERROR: Data boundary error (attempted DMA across 64K boundary).", nl, 0
DiskErrCmd0C      db "ERROR: Unsupported track or invalid media.", nl, 0
DiskErrCmd10      db "ERROR: Uncorrectable CRC or ECC error on read.", nl, 0
DiskErrCmd20      db "ERROR: Controller failure.", nl, 0
DiskErrCmd31      db "ERROR: No media in drive.", nl, 0
DiskErrCmd40      db "ERROR: Seek failed.", nl, 0
DiskErrCmd80      db "ERROR: Timeout (not ready).", nl, 0
DiskErrCmdB0      db "ERROR: Volume not locked in drive.", nl, 0
DiskErrCmdB1      db "ERROR: Volume locked in drive.", nl, 0
DiskErrCmdB2      db "ERROR: Volume not removable.", nl, 0
DiskErrCmdB3      db "ERROR: Volume in use.", nl, 0
DiskErrCmdB4      db "ERROR: Lock count exceeded.", nl, 0
DiskErrCmdB5      db "ERROR: Valid eject request failed.", nl, 0
DiskErrCmdB6      db "ERROR: Volume present but read protected.", nl, 0


SECTION .base


; Save the boot drive number here.
BootDrive         dd 0

Retry             db 0                ; Number of times to retry a particular function - safety purposes.

ALIGN 4
LBAPacket:
   .Size          db 0                ; Size of this packet. Can either be 0x10 or 0x18.
                  db 0                ; Reserved; must not be touched.
   .Sectors       dw 0                ; Number of sectors to read.
   .BufferOff     dw 0                ; Real mode offset to the buffer.
   .BufferSeg     dw 0                ; Real mode segment of the buffer.
   .LBALow        dd 0                ; Lower DWORD of the LBA address to load.
   .LBAHigh       dd 0                ; Higher DWORD of the LBA address to load.


FindPVDMan        db 0                ; Set to 1 if need to find PVD by basic method.


; Initializes the boot disk drive, and finds the Primary Volume Descriptor.
;     @rc
;                 Returns no error code but halts initialization if any error occurs.
InitDisk:
    pushad
    xor si, si                        ; If any error occurs in the future - abort using basic method.
    call CheckInt13Ext                ; Check whether Int 13 extensions are present or not.

    mov edi, 0x9000                   ; Read the following sector at 0x9000.
    mov ecx, 1                        ; Only read 1 sector (2KiB).
    mov ebx, [BootInfo.PVD]           ; Read the sector containing the PVD.
    
    call ReadFromDisk                 ; Read the sector containing the Primary Volume Descriptor.
    jc AbortBoot                      ; Abort boot if read failed.

    cmp byte [0x9000], 1              ; Check whether we loaded a PVD or not.
    jc .Manual                        

    cmp dword [0x9001], 'CD00'        ; Check whether valid sector or not.
    jc .Manual         

    cmp byte [0x9005], '1'            ; Check whether valid sector or not.
    jc .Manual         
    
    popad
    ret

.Manual:
    mov byte [FindPVDMan], 1

    popad
    ret


; Checks the boot file (us), and tries restoring it if (possible and) any error occured.
CheckBootFile:
    pushad
    xor si, si                        ; If any error occurs in the future - abort using basic method.

    mov edi, 0x7C00
    cmp word [di + 510], 0xAA55
    jne AbortBoot                     ; The boot signature at 510 byte offset was incorrect - can't assume anything. ABORT!

    add edi, 0x800                    ; The file is four sectors long as of now.
    cmp dword [di + 2040], "DRAU"     ; Check our Draumr signature.
    jne .FixFile

    cmp dword [di + 2044], "MRSS"     ; Check our Draumr signature.
    je .Return

; Try to fix the boot file.
.FixFile:
    ; Get LBA of boot file.
    mov ebx, [BootInfo.BFLBA]
    ; Load only 1 sector.
    mov ecx, 1

    ; Calculate how many sectors to read.
    mov eax, [BootInfo.BFLength]
    add eax, 0x7FF                    ; Add 0x7FF to remain "safe".
    shr eax, 11                       ; Find out the number of 2KiB sectors.

.LoadSector:
    call ReadFromDisk                 ; Read the sector from disk.

.NextSector: 
    inc ebx                           ; Increment the LBA of sector to read.
    dec eax                           ; Decrement the number of sectors left to read.
    add di, 0x800                     ; Add 0x800 to DI = that is the next sector.
    test eax, eax
    jnz .LoadSector

; Check whether the file on disk is corrupt or not.
.CheckFileAgain:
    mov edi, 0x7C00 + 0x800
    cmp dword [di + 2040], "DRAU"     ; Check out the Draumr signature.
    jne AbortBoot

    cmp dword [di + 2044], "MRSS"     ; Check out the Draumr signature.
    jne AbortBoot

.Return:    
    popad
    ret
 

; Checks whether the BIOS supports Int 0x13 extensions or not.
;     @rc         
;                 Returns no error code, but halts initialization if error occurs.
CheckInt13Ext:
    pushad                            ; Push all general purpose registers to save them.

    mov eax, 0x4100
    mov ebx, 0x55AA
    mov edx, [BootDrive]
    int 0x13                          ; Check whether extended functions are supported or not.
    jc .Error                         ; If carry flag is set, go to return since carry flag is set anyway.
    
    cmp bx, 0xAA55                    ; If BX=0xAA55, extended versions are installed. Return.
    je .Return

    test ah, ah                       ; If AH is zero, the call was successful. Return.
    je .Return                        

.Error:
    xor si, si
    call AbortBoot                    ; Abort boot using base method.
    
.Return:
    popad
    ret                               


; Read a sector from the disk to a buffer.
;     @edi        The buffer to where to read the disk sector to.
;                 If bit 0x80000000 is set, advanced error checking is used.

;     @ebx        The LBA address from where to read to.
;     @ecx        The number of sectors to read.
;     @rc
;                 Aborts boot if unsuccesful.
ReadFromDisk:
    mov byte [Retry], 3
    pushad

.Retry:
    mov byte [LBAPacket.Size], 0x10   ; Size of the Packet - 0x10 if you specify Segment:Offset and not a 64-bit linear address.
    mov word [LBAPacket.Sectors], cx  ; Number of sectors to read.
    mov [LBAPacket.BufferOff], di     ; Offset - Since we use 0x0000 as segment offset must be the address of the buffer - @di.
    mov word [LBAPacket.BufferSeg], 0  
    mov [LBAPacket.LBALow], ebx       ; Lower 32-bits of the LBA address.
    mov [LBAPacket.LBAHigh], dword 0  ; Higher 32-bits of the LBA address - useful for 48-bit LBA.
     
    mov si, LBAPacket
    mov eax, 0x4200
    mov dl, [BootDrive]               ; We just know about one drive as of now - so read that only.
    int 0x13
    
    test ah, ah                       ; If AH is zero, call successful. Return.
    je .Success

.Error:
    push ebx
    mov bl, [Retry]
    dec bl
    mov [Retry], bl
    test bl, bl
    pop ebx
    je .Abort

    jmp .Retry

.Abort:
    xor si, si

    test edi, 0x80000000
    jnz .AdvancedAbort

    call AbortBoot

.AdvancedAbort:
    call GetErrorMsg
    call AbortBoot

.Success:
    popad
    ret


SECTION .text


; Reads multiple sectors from disk.
; Expects same arguments as that for the above.
; M stands for Multiple (sectors).
ReadFromDiskM:
    pushad
   
    ; Save the count of sectors to read in EAX. 
    mov eax, ecx
    ; And read on sector at a time - reading more sectors cause a lot of problems usually.
    mov ecx, 1

.Single:
    call ReadFromDisk
    
    ; Decrease the count of sectors to read.
    dec eax
    ; If read all sectors, end.
    test eax, eax
    jz .Return
    
    inc ebx                           ; Increase the LBA.
    add di, 0x800                     ; And the destination buffer.
    jmp .Single 

.Return:
    popad
    ret


; Prints out error messages reported by the BIOS on disk operations.
; @ah             Is supposed to contain the error code.
;     @rc
;     @si         Contains the address of the string to print (when disk error occured).
;     @ax         The type of beep to produce.
GetErrorMsg:
    test ah, ah                       ; If AH is zero, print a generic error message.
    je .Generic

    xor si, si
    movzx ebx, ah
    xor ax, ax                        ; TODO: This should contain the code for disk error.

.Table1:
    cmp bx, 0x10
    jg .Table2                        ; If AH doesn't fit in table 1, try table 2.

    ; Find the string address in the Disk Error Table.
    add bx, bx                        ; Double AX, since each address is a word and not a byte.
    add bx, DiskErrTable1             ; Add the address of the DiskErrTable1 to it
    
    mov dx, [bx]                      ; Get the address of the string into DX.
 
    ; If DX is zero go to the generic case.
    test dx, dx
    je .Generic

    ; Print the error message.
    mov si, dx
    ret

.Table2:
    cmp bx, 0xB0
    jb .Special20                     ; If it is smaller than 0xB0, we can try the special cases.
    cmp bx, 0xB6
    jg .Generic                       ; Else, the generic ones.

    ; Find the string address in the Disk Error Table.
    sub bx, 0xB0                      ; Subtract 0xB0 to find the offset.
    add bx, bx                        ; Double AX, since each address is a word and not a byte.
    add bx, DiskErrTable2             ; Add the address of the DiskErrTable2 to it
  
    mov dx, [bx]
 
    ; If DX is zero go to the generic case.
    test dx, dx
    je .Generic

    ; Print the error message.
    mov si, dx
    ret

; NOTE: For all cases where I didn't put the error messages in a table
; so as to decrease the size of the table, I test for them in a Special
; Case clause.

; Special case for error code 0x20.
.Special20:
    cmp bx, 0x20
    jne .Special31

    mov si, DiskErrCmd20
    ret

; Special case for error code 0x31.
.Special31:
    cmp bx, 0x31
    jne .Special40

    mov si, DiskErrCmd31
    ret

; Special case for error code 0x40.
.Special40:
    cmp bx, 0x40
    jne .Special80

    mov si, DiskErrCmd40
    ret

; Special case for error code 0x80.
.Special80:
    cmp bx, 0x80
    jne .Generic

    mov si, DiskErrCmd80
    ret

.Generic:
    mov si, DiskErr0
    ret


