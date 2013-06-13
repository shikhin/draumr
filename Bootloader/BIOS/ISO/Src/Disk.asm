 ; Contains functions to access the Disk.
 ;
 ; Copyright (c) 2013, Shikhin Sethi
 ; All rights reserved.
 ;
 ; Redistribution and use in source and binary forms, with or without
 ; modification, are permitted provided that the following conditions are met:
 ;     * Redistributions of source code must retain the above copyright
 ;       notice, this list of conditions and the following disclaimer.
 ;     * Redistributions in binary form must reproduce the above copyright
 ;       notice, this list of conditions and the following disclaimer in the
 ;       documentation and/or other materials provided with the distribution.
 ;     * Neither the name of Draumr nor the
 ;       names of its contributors may be used to endorse or promote products
 ;       derived from this software without specific prior written permission.
 ;
 ; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 ; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 ; DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 ; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 ; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 ; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 ; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

SECTION .data

; The sectors read - used to return from DiskReadSectorM.
SectorRead:       dd 0

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

; This section contains all the Strings.
DiskErr0          db "Read from CD failed.", EL, 0
DiskErr1          db "Not able to find the Primary Volume Descriptor.", EL, 0
DiskErr2          db "Int 0x13 extension not supported.", EL, 0

; Error messages returned by a BIOS in AH during disk operations.
DiskErrCmd01      db "Invalid function in AH or invalid parameter.", EL, 0
DiskErrCmd02      db "Address mark not found.", EL, 0
DiskErrCmd03      db "Disk write-protected.", EL, 0
DiskErrCmd04      db "Sector not found/read error.", EL, 0
DiskErrCmd08      db "DMA overrun.", EL, 0
DiskErrCmd09      db "Data boundary error (attempted DMA across 64K boundary).", EL, 0
DiskErrCmd0C      db "Unsupported track or invalid media.", EL, 0
DiskErrCmd10      db "Uncorrectable CRC or ECC error on read.", EL, 0
DiskErrCmd20      db "Controller failure.", EL, 0
DiskErrCmd31      db "No media in drive.", EL, 0
DiskErrCmd40      db "Seek failed.", EL, 0
DiskErrCmd80      db "Timeout (not ready).", EL, 0
DiskErrCmdB0      db "Volume not locked in drive.", EL, 0
DiskErrCmdB1      db "Volume locked in drive.", EL, 0
DiskErrCmdB2      db "Volume not removable.", EL, 0
DiskErrCmdB3      db "Volume in use.", EL, 0
DiskErrCmdB4      db "Lock count exceeded.", EL, 0
DiskErrCmdB5      db "Valid eject request failed.", EL, 0
DiskErrCmdB6      db "Volume present but read protected.", EL, 0

SECTION .base

; Error if the boot file is found to be corrupt.
ErrorBootFileMsg:
    db "The boot file (Stage1) is corrupt.", EL, 0

ErrorPVDMsg:
    db "Unable to find the PVD - ISO corrupt.", EL, 0

ErrorInt13ExtMsg:
    db "Int 13 extensions not present.", EL, 0

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

 ; Initializes the boot disk drive, and finds the Primary Volume Descriptor.
 ;
 ; Returns:
 ;     Boot -> aborted if any error occurs.
DiskInit:
    ; Save some registers.
    push si
    push edi
    push ecx
    push ebx

    call Int13ExtCheck                ; Check whether Int 13 extensions are present or not.

    mov edi, 0x9000                   ; Read the following sector at 0x9000.
    mov ecx, 1                        ; Only read 1 sector (2KiB).
    mov ebx, [BootInfo.PVD]           ; Read the sector containing the PVD.
    
    call DiskReadSector               ; Read the sector containing the Primary Volume Descriptor.
    jc .ErrorPVD                      ; Abort boot if read failed.

    cmp byte [0x9000], 1              ; Check whether we loaded a PVD or not.
    jne .ErrorPVD                        

    cmp dword [0x9001], 'CD00'        ; Check whether valid sector or not.
    jne .ErrorPVD         

    cmp byte [0x9005], '1'            ; Check whether valid sector or not.
    jne .ErrorPVD         
    
    jmp .Return

.ErrorPVD:
    mov si, ErrorPVDMsg
    jmp AbortBoot

.Return:
    ; Restore the registers.
    pop ebx
    pop ecx
    pop edi
    pop si
    
    ret

 ; Checks the boot file (us), and tries restoring it if (possible and) any error occured.
BootFileCheck:
    pushad

    cmp word [0x7E00 - 2], 0xAA55
    jne .ErrorBootFile                 ; The boot signature at 510 byte offset was incorrect.

    cmp dword [0x8E00 - 8], "DRAU"     ; Check out the Draumr signature.
    jne .ErrorBootFile

    cmp dword [0x8E00 - 4], "MRSS"     ; Check out the Draumr signature.
    je .Return

.ErrorBootFile:
    mov si, ErrorBootFileMsg
    jmp AbortBoot

.Return:    
    popad
    ret
 
 ; Checks whether the BIOS supports Int 0x13 extensions or not.
 ;
 ; Returns:         
 ;     Boot -> aborted if any error occurs.
Int13ExtCheck:
    pushad                            ; Push all general purpose registers to save them.

    mov eax, 0x4100
    mov ebx, 0x55AA
    mov edx, [BootDrive]
    int 0x13                          ; Check whether extended functions are supported or not.

    jc .ErrorInt13Ext                 ; If carry flag is set, go to abort since carry flag is set anyway.
    
    cmp bx, 0xAA55                    ; If BX=0xAA55, extended versions are installed. Return.
    je .Return

.ErrorInt13Ext:
    mov si, ErrorInt13ExtMsg
    jmp AbortBoot
   
.Return:
    popad
    ret                               

 ; Read a sector from the disk to a buffer.
 ;     EDI  -> the buffer to where to read the disk sector to.
 ;          -> bit 0x80000000 indicates advanced error checking is used.
 ;     EBX  -> the LBA address from where to read to.
 ;     ECX  -> the number of sectors to read.
 ;
 ; Returns:
 ;     Boot -> aborted if any error occurs.
DiskReadSector:
    pushad

    ; Retry 3 times before failing.
    mov byte [Retry], 3
    mov ebp, edi

    ; Get rid of the "perform advanced checking" flag, since it is already stored at in ebp.
    and edi, 0x7FFFFFFF
    mov eax, edi
    and eax, 0xFFFF0000
    shr eax, 4
    ; Get the segment in AX.

.Retry:
    mov byte [LBAPacket.Size], 0x10   ; Size of the Packet - 0x10 if you specify Segment:Offset and not a 64-bit linear address.
    mov word [LBAPacket.Sectors], cx  ; Number of sectors to read.
    mov [LBAPacket.BufferOff], di     ; Offset.
    mov word [LBAPacket.BufferSeg], ax; Get the segment. 
    mov [LBAPacket.LBALow], ebx       ; Lower 32-bits of the LBA address.
    mov [LBAPacket.LBAHigh], dword 0  ; Higher 32-bits of the LBA address - useful for 48-bit LBA.
     
    mov si, LBAPacket
    mov eax, 0x4200
    mov dl, [BootDrive]               ; We just know about one drive as of now - so read that only.
    int 0x13
   
    jnc .Success

.Error:
    dec byte [Retry]
    jz .Abort

    clc
    jmp .Retry

.Abort:
    push ax
    ; Set to mode 0x03, or 80*25 text mode.
    mov ax, 0x03
   
    ; SWITCH!
    int 0x10
    pop ax
 
    mov si, DiskErr0

    test ebp, 0x80000000
    jz AbortBoot
    
.AdvancedAbort:
    call DiskErrorMsg
    jmp AbortBoot

.Success:
    popad
    ret

SECTION .text

 ; Reads multiple sectors from disk.
 ;     EDI  -> the buffer to where to read the disk sector to.
 ;          -> bit 0x80000000 indicates advanced error checking is used.
 ;     EBX  -> the LBA address from where to read to.
 ;     ECX  -> the number of sectors to read.
 ;
 ; Returns:
 ;     ECX  -> the number of sectors actually read.
; NOTE: M stands for multiple sectors.
DiskReadSectorM:
    pushad
   
    mov ebp, edi

    ; Get rid of the "perform advanced checking" flag, since it is 
    ; already stored at in @ebp.
    and edi, 0x7FFFFFFF
    mov eax, edi
    and eax, 0xFFFF0000
    shr eax, 4
    ; Get the segment in AX.

    cmp ecx, 0x7F                     ; 0x7F is the maximum sectors we can read, so, let's minimize this number.
    jbe .Multiple                     ; If below or equal, do multiple sector reads.

    mov ecx, 0x7F                     ; If count is > 0x7F, then count = 0x7F;

.Multiple:
    mov byte [LBAPacket.Size], 0x10   ; Size of the Packet - 0x10 if you specify Segment:Offset and not a 64-bit linear address.
    mov word [LBAPacket.Sectors], cx  ; Number of sectors to read.
    mov [LBAPacket.BufferOff], di     ; Offset - Since we use 0x0000 as segment offset must be the address of the buffer - @di.
    mov word [LBAPacket.BufferSeg], ax; The segment is zero. 
    mov [LBAPacket.LBALow], ebx       ; Lower 32-bits of the LBA address.
    mov [LBAPacket.LBAHigh], dword 0  ; Higher 32-bits of the LBA address - useful for 48-bit LBA.
     
    mov si, LBAPacket
    mov eax, 0x4200
    mov dl, [BootDrive]               ; We just know about one drive as of now - so read that only.
    int 0x13
   
    jc .Single                        ; If we failed, try the single by single method.

    cmp word [LBAPacket.Sectors], 0   ; Let's check if the sectors read is zero - if zero, move to single.
    je .Single                        ; If zero, move to single.

    movzx ecx, word [LBAPacket.Sectors]
    mov [SectorRead], ecx             ; Let's store the number of sectors read in SectorRead.
    jmp .Return

.Single:
    mov [SectorRead], ecx             ; Sectors read would be ECX in any case - since we'd read all sectors anyway (single-single)

    ; Save the count of sectors to read in EAX. 
    mov eax, ecx
    ; And read one sector at a time - the recommended method.
    mov ecx, 1
   
    ; Restore the original ebp.
    mov edi, ebp
    
.Loop:
    call DiskReadSector
    
    ; Decrease the count of sectors to read.
    ; If read all sectors, end.
    dec eax
    jz .Return
    
    inc ebx                           ; Increase the LBA.
    add di, 0x800                     ; And the destination buffer.
    jmp .Loop

.Return:
    popad
    mov ecx, [SectorRead]             ; We are going to return the sectors read in ECX - so get it!
    ret

 ; Prints out error messages reported by the BIOS on disk operations.
 ;     AH -> the error code.
 ;
 ; Returns:
 ;     SI -> the address of the string to print.
DiskErrorMsg:
    test ah, ah                       ; If AH is zero, print a generic error message.
    je .Generic

    xor si, si
    movzx ebx, ah

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