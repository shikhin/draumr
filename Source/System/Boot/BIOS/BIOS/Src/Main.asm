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
    .OpenFile     dd 0
    .ReadFile     dd 0
    .CloseFile    dd 0
    .HrdwreFlags  db 0                ; The "hardware" flags.
    .VideoFlags   db 0                ; The video card information flags.
  
    .ACPI         dd 0                ; The 32-bit address of the RSDP.
    .MPS          dd 0                ; The 32-bit address of the MPS tables.
    .SMBIOS       dd 0                ; The 32-bit address of the SMBIOS tables.

; Hardware flags.
%define A20_DISABLED    (1 << 0)

; Video flags.
%define VBE_PRESENT     (1 << 0)      ; Describes whether VBE was present or not.

SECTION .text
GLOBAL Startup

; Point where the Stage 1 boot loader handles control.
; @ss:sp          Should point to 0x7C00.
; @eax            Should point to the OpenFile function.
; @ebx            Should point to the ReadFile function.
; @ecx            Should point to the CloseFile function.
Startup:
    mov [BIT.OpenFile], eax
    mov [BIT.ReadFile], ebx
    mov [BIT.CloseFile], ecx
    
    ; Enable A20, then try to generate memory map.
    call EnableA20
    call MMapBuild
    call VideoInfoBuild
   
    ; Switch to protected mode, and go to .Protected32 label.
    mov ebx, .Protected32
    call SwitchToPM
   
BITS 32
.Protected32:
    call FindTables

    ; Enable paging.
    call EnablePaging

.Die:
    hlt 
    jmp .Die

BITS 16

%include "Source/System/Boot/BIOS/BIOS/Src/Memory.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/A20.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Video/Video.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Tables/Tables.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Paging.asm"

SECTION .text

; Performs a switch to protected mode - making sure to save all registers (except segment one - of course).
SwitchToPM:
    cli
 
    lgdt [GDTR32]                     ; Load the GDT.
    
    mov eax, cr0                      ; Or 1 with CR0, to enable the PM bit.
    or al, 1
    mov cr0, eax

    jmp 0x08:.Switched                ; Reload the code segment register.

; 32-bit mode here.
BITS 32
.Switched:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax 
    mov ss, ax                        ; Reload all the other segment registers too.

.Return:
    jmp ebx

; Switch to Real mode back for future generations.
SwitchToRM:
    lgdt [GDTR16]                     ; Load the 16-bit GDT.

    jmp 0x08:.Protected16             ; And jump into 16-bit protected mode!

; Now, back to 16-bits.
BITS 16
.Protected16:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov eax, cr0                      ; Switch off protected mode.
    and eax, ~1
    mov cr0, eax 

    jmp 0x00:.RealMode

.RealMode:
    mov ax, 00
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov ss, ax

    sti
    
.Return:
    jmp ebx

SECTION .data
; The GDTR, which is loaded in the GDTR register.
GDTR32:
    dw (0x08 * 3) - 1                 ; It's the size of all entries, minus 1.
    dd GDT32

; And the actual GDT here.
GDT32:
    dd 0x00000000, 0x00000000         ; The null entry.

    ; The code entry - limit is 0xFFFF, base is 0x0000.
    dw 0xFFFF, 0x0000                 
    db 0x00, 0x9A, 0xCF, 0x00         ; The base, access, flags and limit byte.

    ; The data entry.
    dw 0xFFFF, 0x0000                 
    db 0x00, 0x92, 0xCF, 0x00         ; The base, access, flags and limit byte.

; The GDTR, which is loaded in the GDTR register.
GDTR16:
    dw (0x08 * 3) - 1                 ; It's the size of all entries, minus 1.
    dd GDT16

; And the actual GDT here.
GDT16:
    dd 0x00000000, 0x00000000         ; The null entry.

    ; The code entry - limit is 0xFFFF, base is 0x0000.
    dw 0xFFFF, 0x0000                 
    db 0x00, 0x9A, 0x0F, 0x00         ; The base, access, flags and limit byte.

    ; The data entry.
    dw 0xFFFF, 0x0000                 
    db 0x00, 0x92, 0x0F, 0x00         ; The base, access, flags and limit byte.


