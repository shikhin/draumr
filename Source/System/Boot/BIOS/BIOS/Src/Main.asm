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
  
    .ACPI         dd 0                ; The 32-bit address of the RSDP.
    .MPS          dd 0                ; The 32-bit address of the MPS tables.
    .SMBIOS       dd 0                ; The 32-bit address of the SMBIOS tables.

    .MMap         dd 0                ; The 32-bit address of the MMap.

    ; BIT Video stuff here.
    .VideoFlags      db 0             ; The "video" flags.
    .VBECntrlrInfo   dd 0             ; The 32-bit address of the VBE Controller Info block.
    .VBEModeInfo     dd 0             ; The 32-bit address of the VBE Mode Info block.
    .VBEModeInfoN    dd 0             ; The number of entries in the VBE Mode Info block.

    .SwitchVGA       dd 0             ; The 32-bit address of the function to switch to a VGA mode.
    .SetupPaletteVGA dd 0             ; The 32-bit address of the function to set up the palette in 8bpp modes.
    .GetModeInfoVBE  dd 0             ; The 32-bit address of the function to get mode information from VBE.

; Put all the real-mode functions to handle files here.
OpenFile dd 0
ReadFile dd 0
CloseFile dd 0

; Hardware flags.
%define A20_DISABLED    (1 << 0)

; Video flags.
%define VGA_PRESENT     (1 << 0)
%define VBE_PRESENT     (1 << 1)

; Abort boot if can't open file.
ErrorFile db "ERROR: Error occured while trying to open file.", 0

; Or file is incorrect.
ErrorBIOSFile db "ERROR: Error occured while trying to parse the DBAL file.", 0

; Or we are trying to close a file which we haven't even opened.
ClosingWhileNotOpened db "ERROR: Trying to close a file, while none has been opened yet.", 0

SECTION .text
GLOBAL Startup

; Point where the Stage 1 boot loader handles control.
; @ss:sp          Should point to 0x7C00.
; @eax            Should point to the OpenFile function.
; @ebx            Should point to the ReadFile function.
; @ecx            Should point to the CloseFile function.
Startup:
    mov [OpenFile], eax
    mov [ReadFile], ebx
    mov [CloseFile], ecx

    ; I might do this statically, but, I may change this in the future from here - better idea (and doesn't take THAT much time).
    mov dword [BIT.SwitchVGA], SwitchVGAWrapper
    mov dword [BIT.SetupPaletteVGA], SetupPaletteVGAWrapper
    mov dword [BIT.GetModeInfoVBE], GetModeInfoVBEWrapper
    mov dword [BIT.OpenFile], OpenFileWrapper
    mov dword [BIT.ReadFile], ReadFileWrapper
    mov dword [BIT.CloseFile], CloseFileWrapper

    ; Enable A20, then try to generate memory map.
    call EnableA20
    call MMapBuild
    call VideoInit

.LoadDBAL:    
    xor ax, ax                        ; Open File 1, or DBAL file.
    inc ax
    
    call word [OpenFile]             ; Open the File.
    
    jc .Error
    
    ; ECX contains size of file we are opening.
    push ecx
    mov ecx, 512                      ; Read only 512 bytes.
   
    mov edi, 0xE000
    call [ReadFile]                   ; Read the entire file.

.CheckDBAL1:
    cmp dword [0xE000], "DBAL"        ; Check the signature.

    jne .Error2

    mov ecx, [0xE000 + 16]            ; Get the end of file in ECX - actual file size.
    sub ecx, 0xE000                   ; Subtract 0xE000 from it to get it's size.
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
    
    call word [ReadFile]             ; Read the rest of the file.
    
.Finish:
    call word [CloseFile]            ; And then close the file.

    ; Switch to protected mode - since we might be crossing our boundary here.
    mov ebx, .CheckDBAL2
    call SwitchToPM

BITS 32
.CheckDBAL2:
    mov ecx, [0xE000 + 16]            ; Get the end of the file in ECX.
    sub ecx, 0xE000 + 24              ; Subtract 0xE000 (address of start) + 24 (size of header) from it, to get the size.
    
    mov esi, 0xE000 + 24              ; Calculate CRC from above byte 18.    
    mov eax, 0xFFFFFFFF               ; Put the seed in EAX.
    
    call CRC32
    
    not eax                           ; Inverse the bits to get the CRC value.
    cmp eax, [esi - 4]                ; Compare the has with the hash stored in the file.
        
    je .ZeroBSS
    
    ; If error occured, switch to Real Modee
    mov ebx, .Error2
    call SwitchToRM

.ZeroBSS:
    mov esi, 0xE000 
    mov edi, [esi + 8]                ; Move the start of BSS section into EDI.
   
    mov ecx, [esi + 12]
    sub ecx, edi                      ; Calculate the length, and store it in ECX.

    xor eax, eax                      ; Zero out EAX, since we want to clear the region.
    rep stosb                         ; Clear out the BSS section.

    jmp .Protected32

BITS 16
   
.Error:
    mov si, ErrorFile
    jmp AbortBoot

.Error2:
    mov si, ErrorBIOSFile
    jmp AbortBoot

BITS 32
.Protected32:
    call FindTables
   
; Jump to the DBAL file here.
.JmpToDBAL:
    ; Reset the stack - who needs all the junk anyway?
    mov esp, 0x7C00
    ; Store the address of the BIT in the EAX register - we are going to be needing it later on.
    mov eax, BIT
    
    call word [0xE004]

BITS 16

SECTION .text

BITS 32
%include "Source/System/Boot/Lib/CRC32/CRC32.asm"
BITS 16
%include "Source/System/Boot/BIOS/BIOS/Src/Memory.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Screen.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Abort.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/A20.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Video/Video.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Tables/Tables.asm"

SECTION .text

BITS 32
; A wrapper to the SwitchVGA function - to be done from 32-bit code.
; Argument pushed                     A 16-byte word, defining the mode to switch to.
SwitchVGAWrapper:
    push ebx
   
    mov ebx, .GetInfo
    jmp SwitchToRM                    ; Switch to Real mode, and return to GetInfo.

BITS 16
.GetInfo:
    mov ax, [esp + 8]                 ; Since we pushed EBX earlier, add 8 instead of 4 to get the argument.
    call SwitchVGA                    ; Switch to the VGA mode defined.

    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop ebx
    ret

; A wrapper to the SetupPaletteVGA function - to be done from 32-bit code.
SetupPaletteVGAWrapper:
    push ebx
   
    mov ebx, .SetupPalette
    jmp SwitchToRM                    ; Switch to Real mode, and return to SetupPalette.

BITS 16
.SetupPalette:
    call SetupPaletteVGA              ; Set up the palette.

    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop ebx
    ret

; A wrapper to the GetModeInfoVBE function - to be done from 32-bit code.
; Argument pushed                     A 32-bit dword, defining the address where to write.
;     rc
;                                     The number of entries in @eax.
GetModeInfoVBEWrapper:
    push ebx
   
    mov ebx, .GetModeInfoVBE
    jmp SwitchToRM                    ; Switch to Real mode, and return to GetModeInfoVBE.

BITS 16
.GetModeInfoVBE:
    mov eax, [esp + 8]                ; Get the address into EAX.
    mov [BIT.VBEModeInfo], eax

    call GetModeInfoVBE               ; Get mode information from VBE.
    push eax

    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop eax
    pop ebx
    ret

; A wrapper to the OpenFile function - to be done from 32-bit code.
; Argument pushed                     A 32-bit dword, defining the "code" of the file to open.
;     rc
;                                     The size of the file opened in @eax.
OpenFileWrapper:
    push ebx
   
    mov ebx, .OpenFile
    jmp SwitchToRM                    ; Switch to Real mode, and return to OpenFile.

BITS 16
.OpenFile:
    mov eax, [esp + 8]                ; Get the "file code" into EAX.
    
    call word [OpenFile]              ; Open the file.   
   
    mov eax, ecx                      ; Get the size into @eax.
    jnc .BackToPM

    ; And if we failed for some reason, the size is 0.
    xor eax, eax
    clc
    
.BackToPM:
    push eax
    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop eax  
    pop ebx
    ret


; A wrapper to the ReadFile function - to be done from 32-bit code.
; Argument pushed                     A 32-bit dword, defining the length to read.
;                                     A 32-bit dword, defining the address to read to.
;     rc
;                                     The size of the file opened in @eax.
ReadFileWrapper:
    push ebx
   
    mov ebx, .ReadFile
    jmp SwitchToRM                    ; Switch to Real mode, and return to ReadFile.

BITS 16
.ReadFile:
    mov ecx, [esp + 12]                ; Get the length into EAX.
    mov edi, [esp + 8]                 ; And the address into EDI.
    
    call word [ReadFile]              ; Read the file.
    
    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop ebx
    
    ret


; A wrapper to the CloseFile function - to be done from 32-bit code.
CloseFileWrapper:
    push ebx
   
    mov ebx, .CloseFile
    jmp SwitchToRM                    ; Switch to Real mode, and return to CloseFile.

BITS 16
.CloseFile:
    call word [CloseFile]            ; Close the file.
    jnc .BackToPM

    ; And if we failed for some reason, we got to abort boot.
    mov si, ClosingWhileNotOpened
    jmp AbortBoot
    
.BackToPM:
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
