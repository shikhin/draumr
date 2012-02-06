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

BITS 32

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
    mov eax, [esp + 8]

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
    cmp eax, FILE_CLOSE
    jne .Return

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

    ; TODO: Fix this - what happens if we have switched to a video mode?
    jc Startup.ErrorIO                ; If any error occurs, print a error message and abort boot.
    xor eax, eax                      ; Clear out EAX, since we don't want to return anything anyway.

.ReturnToPM:
    push eax

    mov ebx, .Return
    jmp PMSwitch                     ; And switch back to protected mode for the return.

BITS 32
.Return:
    ; Pop 'eax' containing the return value.
    pop eax

    ; Pop 'edi' - which we push for the 'FileRead' case to make the code uniform.
    pop edi

    ; And 'ebx', which we use for the continuing address by *Switch functions.
    pop ebx
    ret

BITS 16