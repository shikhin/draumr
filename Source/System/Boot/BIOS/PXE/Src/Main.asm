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
%include "Source/System/Boot/BIOS/PXE/Src/PXE/Disk.asm"

SECTION .data

Finish db "Finish", nl, 0

; Prepare the PXENV_GET_CACHED_INFO structure.
PXENV_GET_CACHED_INFO:
    .Status       dw 0
    .PacketType   dw 2
    .BufferSize   dw 0
       .BufferOff dw 0                ; A zero over here means it should return the address of it's own buffer.
       .BufferSeg dw 0
    .BufferLimit  dw 0

BIOS db "BIOS"

SECTION .text

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

.GetCachedInfo:
    mov di, PXENV_GET_CACHED_INFO
    mov bx, GET_CACHED_INFO
    
    call UsePXEAPI                    ; Ask for the cached info.
    
    or ax, [PXENV_GET_CACHED_INFO]    ; Get the status into BX.
    test ax, ax
    jnz .Error

    movzx esi, word [PXENV_GET_CACHED_INFO.BufferOff]
    add esi, 20                       ; Put the source offset into ESI.
       
    xor ax, ax
    mov es, ax
    mov edi, SIP                      ; And address of destination in EDI.
    
    mov ax, [PXENV_GET_CACHED_INFO.BufferSeg]
    mov ds, ax                        ; And the segment of source into DS.
    movsd                             ; Save SIP somewhere.
    movsd
 
    xor ax, ax                        ; Set all the segment registers to 0x0000.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov di, 0x9000
    mov si, BIOS
    mov ecx, 4
    call ReadFile

.Die:
    hlt
    jmp .Die

.Error:
    xor ax, ax
    mov es, ax
    mov ds, ax

    mov si, PXEAPIError
    call AbortBoot
