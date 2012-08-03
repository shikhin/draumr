 ; Header definition for KL file.
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

%include "Source/System/Boot/KL/Format/Format.inc"

EXTERN bss
EXTERN end
EXTERN file_end
EXTERN Main

SECTION .header

; Define the KL Header
KL
ENTRY_POINT       Start
FILE_START        0x18000
FILE_END          file_end
BSS_START         bss
BSS_END           end
CRC32_DEFINE

SECTION .text

GLOBAL Start

GLOBAL x86PagingEnable
GLOBAL PAEPagingEnable
GLOBAL AMD64PagingEnable

EXTERN PageDir
EXTERN PDPT
EXTERN PML4

 ; The entry point for the KL sub-module.
 ;     EAX -> the 32-bit address of the BIT.
 ;     ESP -> this should be equal to 0x7C00 - for clearing.
Start:
    push eax
    call Main
    
    ; We shouldn't be returning here.

 ; PAGING RELATED FUNCTIONS.

 ; Enables x86 paging, and jumps to kernel.
x86PagingEnable:
    ; Put the address of the page directory in CR3.
    mov eax, [PageDir]
    mov cr3, eax

    ; Enable paging (PG bit).
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    ; Set the stack.
    mov esp, 0xF0000000

    jmp [0xC0000004]

 ; Enables PAE paging, and jumps to kernel.
PAEPagingEnable:
    ; Put the address of page directory pointer table in CR3.
    mov eax, PDPT
    mov cr3, eax

    ; Enable PAE bit.
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    ; Enable paging (PG bit).
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    ; Set the stack.
    mov esp, 0xF0000000

    jmp [0xC0000004]

 ; Enables AMD64 paging, and jumps to kernel.
AMD64PagingEnable:
    mov eax, [PML4]
    mov cr3, eax

    ; Enable the PAE bit.
    mov eax, cr4
    or eax, (1 << 5)
    mov cr4, eax

    ; Enable long mode bit in EFER MSR.
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)
    wrmsr

    ; Enable paging (PG bit).
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Switch to 64-bit mode from compatibility mode.
    lgdt [GDT64.Pointer]
    jmp 0x08:.AMD64

.AMD64:
    ; mov ax, 0x10
    ; mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax

    ; mov rsp, 0xFFFF802000000000
    ; mov rax, 0xFFFF800000000004
    ; jmp [rax]
    dw 0xB866, 0x0010, 0xD88E, 0xC08E, 0xE08E, 0xE88E, 0xBC48, 0x0000, 0x0000, 0x8020, 0xFFFF, 0xB848, 0x0004, 0x0000, 0x8000, 0xFFFF, 0x20FF

GDT64:
    .Null: dd 0x00000000, 0x00000000

    .Code: dd 0x00000000
           db 0x00, 0x98, 0x20, 0x00

    .Data: dd 0x00000000
           db 0x00, 0x90, 0x00, 0x00

    .Pointer:
           dw ($ - GDT64) - 1
           dd GDT64, 0x00000000
