; Entry point for PXE Bootloader Stage 1
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


; Entry point where the PXE hands control.
; @cs:ip          Expects CS:IP to point to the linear address 0x7C00.
; @es:bx          Expects it to point to the PXENV+ structure.
; @ss:sp + 4      Expects it to point to the !PXE structure.
GLOBAL Startup
SECTION .text
Startup:
    jmp 0x0000:Main                   ; PXE promises to load the bootloader at 0x0000:0x7C00 However, no trusting
                                      ; anything - so doing a far jump to reload this value
                                      ; to a standard 0x0000:0xIP.

%include "Source/System/Boot/BIOS/PXE/Src/Screen.asm"
%include "Source/System/Boot/BIOS/PXE/Src/Abort.asm"
%include "Source/System/Boot/BIOS/PXE/Src/PXE/PXE.asm"

Finish db "Finish", nl, 0

Main:
    cli                               ; Stop maskable interrupts till a proper stack is set up.
    cld                               ; Clear the direction flag, so that increments happen in string instructions.

    push es
    mov ax, 0xB800                    
    mov es, ax                        ; Save 0xB800 in @es, such that @es:0x0000 points to 0xB8000.
    call InitScreen                   ; Initialize the entire screen to blue, and disable the hardware cursor.					
    pop es

    call InitPXE
   
    xor ax, ax                        ; Set all the segment registers to 0x0000.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov si, Finish
    call Print

.Die:
    hlt
    jmp .Die

