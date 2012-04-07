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
 ;     * Neither the name of Draumr nor the
 ;       names of its contributors may be used to endorse or promote products
 ;       derived from this software without specific prior written permission.
 ;
 ; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 ; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 ; DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
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

ALIGN 4

BIT:
    .FileAPI      dd FileAPI          ; The file API entrance point.
    
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

    .VideoAPI        dd VideoAPI      ; The video API entrance point.
    
; Put all the real-mode functions to handle files here.
FileOpen:          dd 0
FileRead:          dd 0
FileClose:         dd 0

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
%include "Source/System/Boot/BIOS/BIOS/Src/PM.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/API.asm"

BITS 32

%include "Source/Lib/CRC32/CRC32.asm"

BITS 16

%include "Source/System/Boot/BIOS/BIOS/Src/Video/Video.asm"
%include "Source/System/Boot/BIOS/BIOS/Src/Tables/Tables.asm"

GLOBAL Startup

 ; Point where the Stage 1 boot loader handles control.
 ;     SS:SP -> the linear address 0x7C00.
 ;     EAX   -> the FileOpen function.
 ;     EBX   -> the FileRead function.
 ;     ECX   -> the FileClose function.
 ;     EDX   -> the BD flags.
Startup:
    mov [FileOpen], eax
    mov [FileRead], ebx
    mov [FileClose], ecx
    mov [BIT.BDFlags], edx

    ; Enable A20, then try to generate memory map.
    call A20Enable
    call MMapBuild
    call VideoInit
    call IPSCalculate

.LoadDBAL:    
    xor ax, ax                        ; Open File 1, or DBAL file.
    inc ax
    
    call [FileOpen]                   ; Open the File.
    jc .ErrorIO
    
    ; ECX contains size of file we are opening.
    push ecx
    
    mov ecx, 512                      ; Read only 512 bytes.
    mov edi, 0xE000
    call [FileRead]                   ; Read the entire file.

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
   
    call [FileRead]                   ; Read the rest of the file.
    
.Finish:
    call [FileClose]                  ; And then close the file.

    ; Switch to protected mode - since we might be crossing our boundary here.
    mov ebx, .CheckDBAL2
    call PMSwitch

BITS 32
.CheckDBAL2:   
    mov ecx, [0xE000 + 12]            ; Get the end of the file in ECX.
    mov esi, 0xE000 + 28              ; Calculate CRC from above byte 24.
    
    sub ecx, esi                      ; Subtract 0x9000 (address of start) + 24 (size of header) from it, to get the size.
    mov eax, 0xFFFFFFFF               ; Put the seed in EAX.
    
    call CRC32

    not eax                           ; Inverse the bits to get the CRC value.
    cmp eax, [esi - 4]                ; Compare the has with the hash stored in the file.
            
    je .BSSZero
    
    ; If error occured, switch to Real Modee
    mov ebx, .ErrorParse
    call RMSwitch

.BSSZero:
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
    call TablesFind

; Jump to the DBAL file here.
.JmpToDBAL:
    ; Reset the stack - who needs all the junk anyway?
    mov esp, 0x7C00
    ; Store the address of the BIT in the EAX register - we are going to be needing it later on.
    mov eax, BIT

    call [0xE004]

SECTION .pad
    db "BIOS"