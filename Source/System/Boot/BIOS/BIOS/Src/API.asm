 ; API to access functions provided by DBAL.
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

BITS 32

SECTION .data

; Unable to close the file.
ErrorFileCloseMsg:
    db "Unable to close a file. Please check that no stray calls to the File API are being made.", EL, 0

SECTION .text

; The API codes for the file access API.
%define FILE_OPEN   0
%define FILE_READ   1
%define FILE_CLOSE  2

 ; The file API, to access File* functions.
 ;     uint32_t -> the API code of the function.
 ;     ...      -> rest of the arguments.
 ;
 ; Returns:
 ;     uint32_t -> the value returned by the function. UNDEFINED if no value needs to be returned.
FileAPI:
    push ebx
    push edi

    ; Compare the API code, and take appropriate steps.
    mov eax, [esp + 12]

    cmp eax, FILE_CLOSE
    ja .Return

.TryOpen:
    cmp eax, FILE_OPEN
    jne .TryRead

    mov ebx, .FileOpen
    jmp RMSwitch

.TryRead:
    cmp eax, FILE_READ
    jne .TryClose

    mov ebx, .FileRead
    jmp RMSwitch

.TryClose:
    mov ebx, .FileClose
    jmp RMSwitch

BITS 16
; Open File:
.FileOpen:
    mov eax, [esp + 16]               ; Get the "file code" into EAX.
    
    call word [FileOpen]              ; Open the file.   
   
    mov eax, ecx                      ; Get the size into @eax.
    jnc .ReturnToPM

    ; And if we failed for some reason, the size is 0.
    xor eax, eax
    clc
    
    jmp .ReturnToPM

; Read File:
.FileRead:
    mov ecx, [esp + 20]               ; Get the length into EAX.
    mov edi, [esp + 16]               ; And the address into EDI.
    
    call word [FileRead]              ; Read the file.
    
    xor eax, eax                      ; Clear out EAX, since we don't want to return anything anyway.
    jmp .ReturnToPM                   ; And switch back to protected mode for the return.

; Close File:
.FileClose:
    call word [FileClose]             ; Close the file.

    jc .ErrorFileClose                ; If any error occurs, print a error message and abort boot.
    xor eax, eax                      ; Clear out EAX, since we don't want to return anything anyway.

.ReturnToPM:
    push eax

    mov ebx, .PopEax
    jmp PMSwitch                      ; And switch back to protected mode for the return.

.ErrorFileClose:
    ; Switch to a text mode to ensure that we aren't in a video mode.
    mov ax, 0x03
    call VGASwitchMode

    mov si, ErrorFileCloseMsg
    jmp AbortBoot

BITS 32
.PopEax:
    ; Pop 'eax' containing the return value.
    pop eax

.Return:
    ; Pop 'edi' - which we push for the 'FileRead' case to make the code uniform.
    pop edi

    ; And 'ebx', which we use for the continuing address by *Switch functions.
    pop ebx
    ret


; The API codes for the video handling API.
%define VIDEO_VGA_SWITCH_MODE   0
%define VIDEO_VGA_PALETTE       1

%define VIDEO_VBE_SWITCH_MODE   10
%define VIDEO_VBE_PALETTE       11
%define VIDEO_VBE_GET_MODES     12

 ; The video API, to access VGA/VBE* functions.
 ;     uint32_t -> the API code of the function.
 ;     ...      -> rest of the arguments.
 ;
 ; Returns:
 ;     uint32_t -> the value returned by the function. UNDEFINED if no value needs to be returned.
VideoAPI:
    push ebx

    ; Compare the API code, and take appropriate steps.
    mov eax, [esp + 8]

.TryVGA:
    cmp eax, VIDEO_VBE_SWITCH_MODE
    jae .TryVBE

    cmp eax, VIDEO_VGA_PALETTE
    ja .Return

.TryVGASwitchMode:
    cmp eax, VIDEO_VGA_SWITCH_MODE
    jne .TryVGAPalette

    mov ebx, .VGASwitchMode
    jmp RMSwitch

.TryVGAPalette:
    mov ebx, .VGAPalette
    jmp RMSwitch

.TryVBE:
    cmp eax, VIDEO_VBE_GET_MODES
    ja .Return

.TryVBESwitchMode:
    cmp eax, VIDEO_VBE_SWITCH_MODE
    jne .TryVBEPalette

    mov ebx, .VBESwitchMode
    jmp RMSwitch

.TryVBEPalette:
    cmp eax, VIDEO_VBE_PALETTE
    jne .TryVBEGetModes

    mov ebx, .VBEPalette
    jmp RMSwitch

.TryVBEGetModes:
    mov ebx, .VBEGetModes
    jmp RMSwitch

BITS 16
; VGASwitchMode:
.VGASwitchMode:
    mov eax, [esp + 12]               ; Since we pushed EBX earlier, add 12 instead of 8 to get the argument.
    
    call VGASwitchMode                ; Switch to the VGA mode defined.

    xor eax, eax                      ; Clear the return value.
    jmp .ReturnToPM                    ; And switch back to protected mode for the return.

; VGAPalette:
.VGAPalette:
    call VGASetupPalette              ; Set up the palette.

    xor eax, eax                      ; Clear the return value.
    jmp .ReturnToPM                    ; And switch back to protected mode for the return.

; VBEGetModes:
.VBEGetModes:
    mov eax, [esp + 12]               ; Get the address into EAX.
    mov [BIT.VBEModeInfo], eax

    call VBEGetModeInfo               ; Get mode information from VBE.
    
    jmp .ReturnToPM                    ; And switch back to protected mode for the return.

; VBESwitchMode:
.VBESwitchMode:
    mov eax, [esp + 12]               ; Since we pushed EBX earlier, add 12 instead of 8 to get the argument.
    
    call VBESwitchMode                ; Switch to the VBE mode defined.
    
    jmp .ReturnToPM                    ; And switch back to protected mode for the return.

; VBEPalette:
.VBEPalette:
    call VBESetupPalette              ; Set up the palette.
    
    jmp .ReturnToPM                    ; And switch back to protected mode for the return.

.ReturnToPM:
    push eax

    mov ebx, .PopEax
    jmp PMSwitch                     ; And switch back to protected mode for the return.

BITS 32
.PopEax:
    ; Pop 'eax' containing the return value.
    pop eax

.Return:
    ; And 'ebx', which we use for the continuing address by *Switch functions.
    pop ebx
    ret

BITS 16