 ; Entry point for BIOS common file.
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
 ;     * Neither the name of the <organization> nor the
 ;       names of its contributors may be used to endorse or promote products
 ;       derived from this software without specific prior written permission.
 ;
 ; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 ; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 ; DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 ; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 ; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 ; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 ; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

BITS 16

%include "Source/System/Boot/BIOS/BIOS/Format/Format.inc"

SECTION .header
; Define the Common BIOS File Header
EXTERN bss_start
EXTERN bss_end
EXTERN file_end

COMMON_BIOS
ENTRY_POINT       Startup
FILE_START        0x9000
FILE_END          file_end
BSS_START         bss_start
BSS_END           bss_end
CRC32_DEFINE

SECTION .data

BIT:
    .OpenFile     dd OpenFileWrapper
    .ReadFile     dd ReadFileWrapper
    .CloseFile    dd CloseFileWrapper
    
    .IPS          dq 0                ; The intstructions executed per second.
    .HrdwreFlags  db 0                ; The "hardware" flags.
    .BDFlags      db 0                ; The boot device flags.
    
    .ACPI         dd 0                ; The 32-bit address of the RSDP.
    .MPS          dd 0                ; The 32-bit address of the MPS tables.
    .SMBIOS       dd 0                ; The 32-bit address of the SMBIOS tables.

    .MMap         dd MMapHeader       ; The 32-bit address of the MMap.

    ; BIT Video stuff here.
    .VideoFlags      db 0             ; The "video" flags.
    .VBECntrlrInfo   dd 0             ; The 32-bit address of the VBE Controller Info block.
    .VBEModeInfo     dd 0             ; The 32-bit address of the VBE Mode Info block.
    .VBEModeInfoN    dd 0             ; The number of entries in the VBE Mode Info block.

    .EDIDInfo        times 128 db 0   ; The EDID information block.

    .SwitchVGA       dd SwitchVGAWrapper        ; The 32-bit address of the function to switch to a VGA mode.
    .SetupPaletteVGA dd SetupPaletteVGAWrapper  ; The 32-bit address of the function to set up the palette in 8bpp modes.
    .GetModeInfoVBE  dd GetModeInfoVBEWrapper	; The 32-bit address of the function to get mode information from VBE.
    
    .SwitchVBE       dd SwitchVBEWrapper        ; The 32-bit address of the function to switch to a VBE mode.
    .SetupPaletteVBE dd SetupPaletteVBEWrapper  ; The 32-bit address of the function to set up the palette in 8bpp modes.

; Put all the real-mode functions to handle files here.
OpenFile:          dd 0
ReadFile:          dd 0
CloseFile:         dd 0

; Hardware flags.
%define A20_DISABLED    (1 << 0)

; Video flags.
%define VGA_PRESENT     (1 << 0)
%define VBE_PRESENT     (1 << 1)
%define GRAPHICAL_USED  (1 << 2)
%define TEXT_USED       (1 << 3)
%define DITHER_DISABLE  (1 << 4)
%define EDID_PRESENT    (1 << 5)

; Abort boot if can't open file.
ErrorIO db "ERROR: Error occured during file Input/Output.", 0

; Or file is incorrect.
ErrorParse db "ERROR: Error occured while trying to parse the DBAL file.", 0

SECTION .text

%include "Source/System/Boot/BIOS/BIOS/Src/Memory.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Screen.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Abort.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/A20.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/IPS.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Video/Video.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Tables/Tables.asm"

GLOBAL Startup

 ; Point where the Stage 1 boot loader handles control.
 ;     SS:SP -> the linear address 0x7C00.
 ;     EAX   -> the OpenFile function.
 ;     EBX   -> the ReadFile function.
 ;     ECX   -> the CloseFile function.
 ;     EDX   -> the BD flags.
Startup:
    mov [OpenFile], eax
    mov [ReadFile], ebx
    mov [CloseFile], ecx
    mov [BIT.BDFlags], edx

    ; Enable A20, then try to generate memory map.
    call EnableA20
    call MMapBuild
    call VideoInit
    call FindIPS

.LoadDBAL:    
    xor ax, ax                        ; Open File 1, or DBAL file.
    inc ax
    
    call [OpenFile]                   ; Open the File.
    jc .ErrorIO
    
    ; ECX contains size of file we are opening.
    push ecx
    
    mov ecx, 512                      ; Read only 512 bytes.
    mov edi, 0xE000
    call [ReadFile]                   ; Read the entire file.

.CheckDBAL1:
    cmp dword [0xE000], "DBAL"        ; Check the signature.
    jne .ErrorParse
    
    cmp dword [0xE008], 0xE000        ; Check the starting address.
    jne .ErrorParse
    
    mov ecx, [0xE000 + 12]            ; Get the end of file in ECX - actual file size.
    sub ecx, 0xE000                   ; Subtract 0xE000 from it to get it's size.
    add ecx, 0x1FF
    shr ecx, 9                        ; Here we have the number of sectors of the file (according to the header).
    
    mov edx, [esp]                    ; And again - c'mon, read the previous comments in the same lines. :|

    add edx, 0x1FF
    shr edx, 9                        ; Here we have the number of sectors of the file (according to the fs).
    
    cmp ecx, edx
    jne .ErrorParse                   ; If they aren't equal, error.
  
.LoadRestFile:
    add edi, 0x200
    pop ecx
    
    cmp ecx, 0x200
    jbe .Finish
    
    sub ecx, 0x200                    ; Read the rest 0x200 bytes.
   
    call [ReadFile]                   ; Read the rest of the file.
    
.Finish:
    call [CloseFile]                  ; And then close the file.

    ; Switch to protected mode - since we might be crossing our boundary here.
    mov ebx, .CheckDBAL2
    call SwitchToPM

BITS 32
.CheckDBAL2:   
    mov ecx, [0xE000 + 12]            ; Get the end of the file in ECX.
    mov esi, 0xE000 + 28              ; Calculate CRC from above byte 24.
    
    sub ecx, esi                      ; Subtract 0x9000 (address of start) + 24 (size of header) from it, to get the size.
    mov eax, 0xFFFFFFFF               ; Put the seed in EAX.
    
    call CRC32
    
    not eax                           ; Inverse the bits to get the CRC value.
    cmp eax, [esi - 4]                ; Compare the has with the hash stored in the file.
            
    je .ZeroBSS
    
    ; If error occured, switch to Real Modee
    mov ebx, .ErrorParse
    call SwitchToRM

.ZeroBSS:
    mov esi, 0xE000 
    mov edi, [esi + 16]               ; Move the start of BSS section into EDI.
   
    mov ecx, [esi + 20]
    sub ecx, edi                      ; Calculate the length, and store it in ECX.
    shr ecx, 2                        ; Shift ecx right by 2, effectively dividing by 4.
    
    xor eax, eax                      ; Zero out EAX, since we want to clear the region.
    rep stosd                         ; Clear out the BSS section.

    jmp .Cont

BITS 16
.ErrorIO:
    mov si, ErrorIO
    jmp AbortBoot

.ErrorParse:
    mov si, ErrorParse
    jmp AbortBoot

BITS 32
.Cont:
    call FindTables

; Jump to the DBAL file here.
.JmpToDBAL:
    ; Reset the stack - who needs all the junk anyway?
    mov esp, 0x7C00
    ; Store the address of the BIT in the EAX register - we are going to be needing it later on.
    mov eax, BIT
    
    call [0xE004]

%include "Source/System/Lib/CRC32/CRC32.asm"

 ; A wrapper to the SwitchVGA function - to be done from 32-bit code.
 ;     uint16_t -> the mode to switch to.
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
 ;     uint32_t -> the address where to write.
 ;
 ; Returns:
 ;     EAX      -> the number of entries.
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

 ; A wrapper to the SwitchVBE function - to be done from 32-bit code.
 ;     uint16_t -> the mode to switch to.
 ;
 ; Returns:
 ;     AX       -> the status of the call to VBE.
SwitchVBEWrapper:
    push ebx
   
    mov ebx, .GetInfo
    jmp SwitchToRM                    ; Switch to Real mode, and return to GetInfo.

BITS 16
.GetInfo:
    mov ax, [esp + 8]                 ; Since we pushed EBX earlier, add 8 instead of 4 to get the argument.
    call SwitchVBE                    ; Switch to the VBE mode defined.
    push eax
    
    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop eax
    pop ebx
    ret

 ; A wrapper to the SetupPaletteVBE function - to be done from 32-bit code.
SetupPaletteVBEWrapper:
    push ebx
   
    mov ebx, .SetupPalette
    jmp SwitchToRM                    ; Switch to Real mode, and return to SetupPalette.

BITS 16
.SetupPalette:
    call SetupPaletteVBE              ; Set up the palette.
    
    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop ebx
    ret

 ; A wrapper to the OpenFile function - to be done from 32-bit code.
 ;     uint32_t -> the "code" of the file to open.
 ;
 ; Returns:
 ;     EAX      -> the size of the file opened.
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
 ;     uint32_t -> the length to read.
 ;     uint32_t -> the address to read to.
 ;
 ; Returns:
 ;     EAX      -> the size of the file opened.
ReadFileWrapper:
    push ebx
    push edi
   
    mov ebx, .ReadFile
    jmp SwitchToRM                    ; Switch to Real mode, and return to ReadFile.

BITS 16
.ReadFile:
    mov ecx, [esp + 16]                ; Get the length into EAX.
    mov edi, [esp + 12]                ; And the address into EDI.
    
    call word [ReadFile]              ; Read the file.
    
    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop edi
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
    jc ErrorIO
    
.BackToPM:
    mov ebx, .Return
    jmp SwitchToPM                    ; And switch back to protected mode for the return.

BITS 32
.Return:
    pop ebx
    ret

BITS 16
 ; Performs a switch to protected mode - making sure to save all registers (except segment one - of course).
 ;     EBX -> the return address here.
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