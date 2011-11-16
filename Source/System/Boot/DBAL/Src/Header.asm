; Header definition for DBAL file.
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

BITS 32

%include "Source/System/Boot/DBAL/Format/Format.inc"

SECTION .header
; Define the Common BIOS File Header
EXTERN bss
EXTERN end
EXTERN file_end
EXTERN Main

DBAL
ENTRY_POINT       Start
BSS_START         bss
BSS_END           end
FILE_END          file_end
CRC32_DEFINE

; Here, we start.
; @eax            Contains the starting of the BIT.
; @esp            This should be equal to 0x7C00 - for clearing.
GLOBAL Start
Start:
    mov ebp, esp

    push eax
    call Main