 ; Entry point for PXE Bootloader Stage 1
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

GLOBAL Startup

SECTION .data

%define BD_CD        0
%define BD_FLOPPY    1
%define BD_PXE       2

; Error in IO.
ErrorIO:
    db "ERROR: Error occured during file Input/Output.", 0

; Error while parsing file.
ErrorParse:
    db "ERROR: Error occured while trying to parse common BIOS File.", 0

; Prepare the PXENV_GET_CACHED_INFO structure.
PXENV_GET_CACHED_INFO:
    .Status       dw 0
    .PacketType   dw 2
    .BufferSize   dw 0
       .BufferOff dw 0                ; A zero over here means it should return the address of it's own buffer.
       .BufferSeg dw 0
    .BufferLimit  dw 0

SECTION .text

 ; Entry point where the PXE hands control.
 ;     CS:IP     -> expects CS:IP to point to the linear address 0x7C00.
 ;     ES:BX     -> expects it to point to the PXENV+ structure.
 ;     ES:SP + 4 -> Expects it to point to the !PXE structure.
Startup:
    jmp 0x0000:Main                   ; PXE promises to load the bootloader at 0x0000:0x7C00 However, no trusting
                                      ; anything - so doing a far jump to reload this value
                                      ; to a standard 0x0000:0xIP.

%include "Source/System/Boot/BIOS/PXE/Src/Screen.asm"
%include "Source/System/Boot/BIOS/PXE/Src/Abort.asm"
%include "Source/System/Boot/BIOS/PXE/Src/PXE/PXE.asm"
%include "Source/System/Boot/BIOS/PXE/Src/PXE/Disk.asm"
%include "Source/System/Lib/CRC32/CRC32.asm"

Main:
    cli                               ; Stop maskable interrupts till a proper stack is set up.

    call ScreenInit                   ; Initialize the entire screen to blue, and disable the hardware cursor.					
    call PXEInit
   
    xor ax, ax                        ; Set all the segment registers to 0x0000.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

.GetCachedInfo:
    mov di, PXENV_GET_CACHED_INFO
    mov bx, GET_CACHED_INFO
    
    call PXEAPICall                   ; Ask for the cached info.
    
    or ax, [PXENV_GET_CACHED_INFO]    ; Get the status into BX.
    jnz .Error

    movzx esi, word [PXENV_GET_CACHED_INFO.BufferOff]
    add esi, 20                       ; Put the source offset into ESI.
       
    mov edi, SIP                      ; And address of destination in EDI.
    
    mov ax, [PXENV_GET_CACHED_INFO.BufferSeg]
    mov ds, ax                        ; And the segment of source into DS.
    movsd                             ; Save SIP somewhere.
    movsd                             ; And GIP.
 
    xor ax, ax
    mov ds, ax
 
.LoadCommonBIOS:
    xor ax, ax                        ; Open File 0, or common BIOS file.
    
    call FileOpen                     ; Open the File.
    jc .ErrorIO

    ; ECX contains size of file we are opening.
    push ecx
    
    mov ecx, 512                      ; Read only 512 bytes.
    mov edi, 0x9000
    call FileRead                     ; Read the entire file.

; Checks common BIOS file from first sector.
.CheckCBIOSFirstSector:
    cmp dword [0x9000], "BIOS"        ; Check the signature.
    jne .ErrorParse

    cmp dword [0x9008], 0x9000        ; Check whether starting address is 0x9000 or not.
    jne .ErrorParse                   ; If no, abort boot.
        
    mov ecx, [0x9000 + 12]            ; Get the end of file in ECX.
    sub ecx, 0x9000                   ; Subtract 0x9000 from it to get it's size.
    add ecx, 0x7FF
    shr ecx, 11                       ; Here we have the number of sectors of the file (according to the header).
 
    mov edx, [esp]                    ; Get the stored ECX.
    
    add edx, 0x7FF
    shr edx, 11                       ; Here we have the number of sectors of the file (according to the fs).

    cmp ecx, edx
    jne .ErrorParse                   ; If they aren't equal, error.

.LoadRestFile:
    add edi, 512
    pop ecx
    
    cmp ecx, 512
    jbe .Finish

    sub ecx, 512                      ; Read the rest bytes.
    
    call FileRead                     ; Read the rest of the file.

.Finish:
    call FileClose                    ; And then close the file.

; Check the rest of the common BIOS file.
.CheckCBIOSRest:
    mov ecx, [0x9000 + 12]            ; Get the end of the file in ECX.
    mov esi, 0x9000 + 28              ; Calculate CRC from above byte 24.
    
    sub ecx, esi                      ; Subtract 0x9000 (address of start) + 24 (size of header) from it, to get the size.
    mov eax, 0xFFFFFFFF               ; Put the seed in EAX.
    
    call CRC32

    not eax                           ; Inverse the bits to get the CRC value.
    cmp eax, [esi - 4]                ; Compare the has with the hash stored in the file.
        
    jne .ErrorParse                   ; Not equal? ERROR: Abort boot.

.ZeroBSS:
    mov esi, 0x9000 
    
    movzx edi, word [esi + 16]        ; Move the start of BSS section into EDI.
    movzx ecx, word [esi + 20]
    
    sub ecx, edi                      ; Calculate the length, and store it in ECX.
    shr ecx, 2                        ; Shift ecx right by 2, effectively dividing by 4.
    
    xor eax, eax                      ; Zero out EAX, since we want to clear the region.
    rep stosd                         ; Clear out the BSS section.
 
.JmpToBIOS:
    xor dx, dx
    mov ss, dx

    mov eax, FileOpen
    mov ebx, FileRead
    mov ecx, FileClose
    mov edx, BD_PXE
       
    mov esp, 0x7C00
    xor ebp, ebp
    jmp [0x9004]

.Error:
    xor ax, ax
    mov es, ax
    mov ds, ax

    mov si, PXEAPIError               ; Accessing the API error occured.
    jmp AbortBoot                     ; Abort boot now!

.ErrorIO:
    mov si, ErrorIO
    jmp AbortBoot                     ; Abort boot now!

.ErrorParse:
    mov si, ErrorParse                ; Unable to parse the common BIOS file.
    jmp AbortBoot
