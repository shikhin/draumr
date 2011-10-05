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
CRC32_DEFINE


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

    .MMap         dd 0                ; The 32-bit address of the MMap.
    .VideoInfo    dd 0                ; The 32-bit address of the Video Information.

    .VBEGetModeInfo dd VBEGetModeInfoWrapper    ; The 32-bit address of the GetModeInfo wrapper.

; Hardware flags.
%define A20_DISABLED    (1 << 0)

; Video flags.
%define VBE_PRESENT     (1 << 0)      ; Describes whether VBE was present or not.

; Abort boot if can't open file.
ErrorFile db "ERROR: Error occured while trying to open file.", 0

; Or file is incorrect.
ErrorBIOSFile db "ERROR: Error occured while trying to parse the DBAL file.", 0

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
    
.LoadDBAL:    
    xor ax, ax                        ; Open File 1, or DBAL file.
    inc ax
    
    call dword [BIT.OpenFile]         ; Open the File.
    
    jc .Error
    
    ; ECX contains size of file we are opening.
    push ecx
    mov ecx, 512                      ; Read only 512 bytes.
   
    mov edi, 0xD000
    call [BIT.ReadFile]               ; Read the entire file.
    
.CheckDBAL1:
    cmp dword [0xD000], "DBAL"        ; Check the signature.

    jne .Error2

    mov ecx, [0xD000 + 10]            ; Get the end of file in ECX - actual file size.
    sub ecx, 0xD000                   ; Subtract 0xD000 from it to get it's size.
    add ecx, 0x1FF
    shr ecx, 9                        ; Here we have the number of sectors of the file (according to the header).
    
    mov edx, [esp]                    ; And again - c'mon, read the previous comments in the same lines. :|

    add edx, 0x1FF
    shr edx, 9                        ; Here we have the number of sectors of the file (according to the fs).
    
    cmp ecx, edx

    jne .Error2                       ; If they aren't equal, error.
  
.LoadRestFile:
    add edi, 0x200
    pop ecx
    
    cmp ecx, 0x200
    jb .Finish

    sub ecx, 0x200                    ; Read the rest 0x200 bytes.
    
    call dword [BIT.ReadFile]         ; Read the rest of the file.
    
.Finish:
    call dword [BIT.CloseFile]        ; And then close the file.

.CheckDBAL2:
    mov ecx, [0xD000 + 10]            ; Get the end of the file in ECX.
    sub ecx, 0xD000 + 18              ; Subtract 0xD000 (address of start) + 18 (size of header) from it, to get the size.
    
    mov esi, 0xD000 + 18              ; Calculate CRC from above byte 18.    
    mov eax, 0xFFFFFFFF               ; Put the seed in EAX.
    
    call CRC32
    
    not eax                           ; Inverse the bits to get the CRC value.
    cmp eax, [esi - 4]                ; Compare the has with the hash stored in the file.
        
    jne .Error2                       ; Not equal? ERROR: Abort boot.

.ZeroBSS:
    mov esi, 0xD000 
    movzx edi, word [esi + 6]         ; Move the start of BSS section into EDI.
   
    movzx ecx, word [esi + 8]
    sub ecx, edi                      ; Calculate the length, and store it in ECX.

    xor eax, eax                      ; Zero out EAX, since we want to clear the region.
    rep stosb                         ; Clear out the BSS section.

    ; Switch to protected mode, and go to .Protected32 label.
    mov ebx, .Protected32
    
    call SwitchToPM
   
.Error:
    xor ax, ax
    mov si, ErrorFile
    call AbortBoot

.Error2:
    xor ax, ax
    mov si, ErrorBIOSFile
    call AbortBoot

BITS 32
.Protected32:
    call FindTables
   
; Jump to the DBAL file here.
.JmpToDBAL:
    ; Reset the stack - who needs all the junk anyway?
    mov esp, 0x7C00
    ; Store the address of the BIT in the EAX register - we are going to be needing it later on.
    mov eax, BIT
    
    call word [0xD004]

BITS 16

SECTION .text

%include "Source/System/Boot/Lib/CRC32/CRC32.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Memory.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Screen.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Abort.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/A20.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Video/Video.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Tables/Tables.asm"

SECTION .text

; Some common wrappers defined here.
AllocatedBuffer dd 0                   ; Temporarily store the address of the buffer.

BITS 32
; A wrapper to the VBEGetModeInfo function - to be done from 32-bit code.
; Argument pushed                     A page aligned allocated area as the output buffer.
VBEGetModeInfoWrapper:
    push ebx
   
    mov ebx, .GetInfo
    jmp SwitchToRM                    ; Switch to Real mode, and return to GetInfo.

BITS 16
.GetInfo:
    mov eax, [esp + 8]                ; Since we pushed EBX earlier, add 8 instead of 4 to get the argument.
    call VBEGetModeInfo               ; Get the mode information.

    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop ebx
    ret

BITS 16
; Performs a switch to protected mode - making sure to save all registers (except segment one - of course).
; @ebx            Would sound dumb - but found no better way (without messing with the stack) - the return address here.
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

SECTION .pad
    db "BIOS"
