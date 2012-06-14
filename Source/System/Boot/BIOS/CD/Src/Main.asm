 ; Entry point for CD Bootloader Stage 1
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

BITS 16
CPU 386

GLOBAL Startup

SECTION .data

%define BD_CD        0
%define BD_FLOPPY    1
%define BD_PXE       2

; End (of) line.
%define EL           0x0A, 0x0D

; The error slogan! (hehe)
ErrorMsg:
    db "ERROR! ERROR! ERROR!", EL, EL, 0

; Abort boot if can't open CBIOS file.
ErrorOpenCBIOSMsg:
    db "Unable to open the common BIOS file.", EL, 0

; Or file is incorrect.
ErrorCBIOSHeaderMsg:
    db "The common BIOS's header has been found to be corrupt.", EL, 0

; Or the CRC value is incorrect.
ErrorCBIOSCRCMsg:
    db "Incorrect CRC32 value of the common BIOS file.", EL, 0

SECTION .base

 ; Entry point where the BIOS hands control.
 ;     DL    -> the drive number.
 ;     CS:IP -> the linear address 0x7C00.
Startup:
    jmp 0x0000:Main                   ; Some BIOS' load the bootloader at 0x0000:0x7C00, while others
                                      ; load it at 0x07C0:0x0000. Do a far jump to reload this value
                                      ; to a standard 0x0000:0xIP.

times 8 - ($-$$) db 0                 ; Pad out the boot information table passed by mkisofs to 8.
BootInfo:
    ; The Boot Information Table passed by mkisofs.
    .PVD          dd 0                ; LBA of the Primary Volume Descriptor
    .BFLBA        dd 0                ; LBA of the Boot File
    .BFLength     dd 0                ; Length of the boot file in bytes
    .Checksum     dd 0                ; 32 bit checksum
    times 40 db 0    

Main:
    cli                               ; Stop maskable interrupts till a proper stack is set up.

    xor ax, ax                        ; Set all the segment registers to 0x0000.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ss, ax
    mov sp, 0x7C00                    ; Set the stack to start from Startup (0x7C00) and continue below.
    
    sti
    
    mov [BootDrive], dl               ; Save @dl which contains the Boot Drive number for future references.
    
    call DiskInit
    call BootFileCheck                ; Check whether the boot file (us) is intact or not.
    
    ; Now, once we have checked whether we are intact or not, and done stuff -
    ; jump to extended main.
    jmp ExtMain

; Pad out the remaining bytes in the first 512 bytes, and then define the boot signature.
    times 510-($-$$) db 0

BootSignature:
    dw 0xAA55

SECTION .text

; Include some files.
%include "Source/Lib/CRC32/CRC32.asm"
%include "Source/System/Boot/BIOS/CD/Src/Abort.asm"
%include "Source/System/Boot/BIOS/CD/Src/Screen.asm"
%include "Source/System/Boot/BIOS/CD/Src/Disk/Disk.asm"
%include "Source/System/Boot/BIOS/CD/Src/Disk/ISO9660.asm"

ExtMain:
    call ScreenInit                   ; Initialize the entire screen to blue, and disable the hardware cursor.					   
    call BootFilesFind                ; Find the boot files.
    
.LoadCommonBIOS:
    xor ax, ax                        ; Open File 0, or common BIOS file.
    
    call FileOpen                     ; Open the File.
    jc .ErrorOpenCBIOS
    
    ; ECX contains size of file we are opening.
    push ecx
    
    mov ecx, 0x800                    ; Read only 0x800 bytes.
    mov edi, 0x9000                   ; At 0x9000.
    call FileRead                     ; Read the first sector.
    
; Check common BIOS file - basic first sector testing.
.CheckCBIOSFirstSector:
    cmp dword [0x9000], "BIOS"        ; Check the signature.
    jne .ErrorCBIOSHeader

    ; If the starting address isn't 0x9000, abort.
    cmp dword [0x9000 + 8], 0x9000
    jne .ErrorCBIOSHeader
    
    mov ecx, [0x9000 + 12]            ; Get the end of file in ECX.
    sub ecx, 0x9000                   ; Subtract 0x9000 from it to get it's size.
    add ecx, 0x7FF
    shr ecx, 11                       ; Here we have the number of sectors of the file (according to the header).
  
    mov edx, [esp]                    ; Get the stored ECX.
    
    add edx, 0x7FF
    shr edx, 11                       ; Here we have the number of sectors of the file (according to the fs).

    cmp ecx, edx
    jne .ErrorCBIOSHeader             ; If they aren't equal, error.
  
; Load the rest of the file.
.LoadRestFile:
    add edi, 0x800
    
    ; Get back the stored ECX.
    pop ecx
    
    ; If we already read the whole file, end.
    cmp ecx, 0x800
    jbe .Finish

    sub ecx, 0x800                    ; Read the rest 0x800 bytes.
    call FileRead                     ; Read the rest of the file.
    
.Finish:
    call FileClose                    ; And then close the file.
   
; Check rest of the common BIOS file.
.CheckCBIOSRest:
    mov ecx, [0x9000 + 12]            ; Get the end of the file in ECX.
    mov esi, 0x9000 + 28              ; Calculate CRC from above byte 28.
    
    sub ecx, esi                      ; Subtract 0x9000 (address of start) + 28 (size of header) from it, to get the size.
    mov eax, 0xFFFFFFFF               ; Put the seed in EAX.

    call CRC32

    not eax                           ; Inverse the bits to get the CRC value.
    cmp eax, [esi - 4]                ; Compare the has with the hash stored in the file.
        
    jne .ErrorCBIOSCRC                ; If not equal, abort boot.

.ZeroBSS:
    mov esi, 0x9000 
    mov edi, [esi + 16]               ; Move the start of BSS section into EDI.
   
    mov ecx, [esi + 20]
    sub ecx, edi                      ; Calculate the length, and store it in ECX.
    shr ecx, 2                        ; Shift the length by 2, effectively dividing by 4.
    
    xor eax, eax                      ; Zero out EAX, since we want to clear the region.
    rep stosd                         ; Clear out the BSS section.

.JmpToBIOS:
    mov eax, FileOpen
    mov ebx, FileRead
    mov ecx, FileClose
    mov edx, BD_CD
       
    ; Reset esp and ebp.
    mov esp, 0x7C00
    xor ebp, ebp

    jmp [0x9004]
    
.ErrorOpenCBIOS:
    mov si, ErrorMsg
    call Print

    mov si, ErrorOpenCBIOSMsg
    jmp AbortBoot

.ErrorCBIOSHeader:
    mov si, ErrorMsg
    call Print

    mov si, ErrorCBIOSHeaderMsg
    jmp AbortBoot

.ErrorCBIOSCRC:
    mov si, ErrorMsg
    call Print

    mov si, ErrorCBIOSCRCMsg
    jmp AbortBoot

SECTION .pad
; Define the DRAUMRSS signature - so that it can be used to check sanity of boot file.
db "DRAUMRSS"