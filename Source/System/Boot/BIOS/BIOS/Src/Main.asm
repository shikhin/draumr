; Entry point for BIOS common file.
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


%include "Source/System/Boot/BIOS/BIOS/Format/Format.inc"


SECTION .header
; Define the Common BIOS File Header
EXTERN bss
EXTERN end
EXTERN file_end

COMMON_BIOS
ENTRY_POINT       Startup
BSS_START         bss
BSS_END           end
FILE_END          file_end
CRC32


SECTION .data
BIT:
    .OpenFile     dw 0
    .ReadFile     dw 0
    .CloseFile    dw 0
    .HrdwreFlags  db 0                ; The "hardware" flags.

%define A20_DISABLED    (1 << 0)

SECTION .text
GLOBAL Startup

; Point where the Stage 1 boot loader handles control.
; @ss:sp          Should point to 0x7C00.
; @eax            Should point to the OpenFile function.
; @ebx            Should point to the ReadFile function.
; @ecx            Should point to the CloseFile function.
Startup:
    mov [BIT.OpenFile], ax
    mov [BIT.ReadFile], bx
    mov [BIT.CloseFile], cx

    ; Enable A20, then try to generate memory map.
    call EnableA20
    call MMapBuild

.Die:
    hlt 
    jmp .Die

%include "Source/System/Boot/BIOS/BIOS/Src/Memory.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/A20.asm"
