; Contains macros defined to "define" the DBAL File Header.
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

; Offset 0        DBAL File Identifier.
; Offset 4        Address of entry point.
; Offset 8        Starting address of BSS section.
; Offset 12       Ending address of BSS section.
; Offset 16       End of file.
; Offset 20       CRC32 remainder.

; This should just be at offset 0 of the file - identifies the file at Common BIOS File.
%define DBAL db "DBAL"

; Defines the entry point of the Common BIOS file.
%macro ENTRY_POINT 1
    dd %1
%endmacro 

; Defines the starting address of BSS section.
%macro BSS_START 1
    dd %1
%endmacro

; Defines the ending address of BSS section.
%macro BSS_END 1
    dd %1
%endmacro

; Defines the end of the file.
%macro FILE_END 1
    dd %1
%endmacro

; The CRC32 remainder.
%define CRC32_DEFINE dd 0
