 ; Header definition for DBAL file.
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
CPU 586

%include "Source/System/Boot/DBAL/Format/Format.inc"

SECTION .header

EXTERN bss
EXTERN end
EXTERN file_end
EXTERN Main
EXTERN CPUCheck
EXTERN BIT

; Define the DBAL Header
DBAL
ENTRY_POINT       Start
FILE_START        0xE000
FILE_END          file_end
BSS_START         bss
BSS_END           end
CRC32_DEFINE

SECTION .text

GLOBAL Start

 ; The entry point for the DBAL sub-module.
 ;     EAX -> the 32-bit address of the BIT.
 ;     ESP -> this should be equal to 0x7C00 - for clearing.
Start:
    ; Check if CPU is supported or not.
    call CPUCheck

    push eax
    call Main

    ; We wouldn't be returning here.

GLOBAL GotoKL

 ; "Jump" to the kernel loader.
 ;     ESP + 4 -> this should contain the address of the entry point.
GotoKL:
    ; Get the address of the entry point.
    mov ebx, [esp + 4]

    ; Preset EAX and ESP for the KL.
    mov eax, BIT
    mov esp, 0x7C00

    ; Call the kernel loader.
    call ebx