; Entry point for CD Bootloader Stage 1
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


BITS 16


; Entry point where the BIOS hands control.
;     @dl         Expects the drive number to be present in dl.
;     @cs:ip      Expects CS:IP to point to the linear address 0x7C00.
GLOBAL Startup
SECTION .base
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


%include "Source/System/Boot/BIOS/CD/Src/Abort.asm"
%include "Source/System/Boot/BIOS/CD/Src/Screen.asm"
%include "Source/System/Boot/BIOS/CD/Src/Disk/Disk.asm"
%include "Source/System/Boot/BIOS/CD/Src/Disk/ISO9660.asm"


SECTION .base
Main:
    cli                               ; Stop maskable interrupts till a proper stack is set up.
    cld                               ; Clear the direction flag, so that increments happen in string instructions.
  
    xor ax, ax                        ; Set all the segment registers to 0x0000.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ss, ax
    mov sp, Startup                   ; Set the stack to start from Startup (0x7C00) and continue below.
    sti
    
    mov [BootDrive], dl               ; Save @dl which contains the Boot Drive number for future references.
   
    call InitDisk
    call CheckBootFile                ; Check whether the boot file (us) is intact or not.

.Exit:			
    jmp ExtMain

; Pad out the remaining bytes in the first 512 bytes, and then define the boot signature.
times 510-($-$$) db 0
dw 0xAA55

SECTION .text
ExtMain:
    push es
    mov ax, 0xB800                    
    mov es, ax                        ; Save 0xB800 in @es, such that @es:0x0000 points to 0xB8000.
    call InitScreen                   ; Initialize the entire screen to blue, and disable the hardware cursor.					
    pop es
    
    cmp byte [FindPVDMan], 1
    jne .FindBootFiles

    call FindPVD                      ; Find the PVD manually, if need be.

.FindBootFiles: 
    call FindBootFiles

.LoadCommonBIOS:
    mov ebx, [BIOS.LBA]
    mov ecx, [BIOS.Size]

    add ecx, 0x7FF
    shr ecx, 11

    mov di, 0x9000
    call ReadFromDiskM

.Die:
    hlt
    jmp .Die

SECTION .pad
; Define the DRAUMRSS signature - so that it can be used to check sanity of boot file.
db "DRAUMRSS"
