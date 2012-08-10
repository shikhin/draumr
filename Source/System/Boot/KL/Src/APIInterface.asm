 ; Contains functions for initializing usage of the PXE API.
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

EXTERN OldAbortBootServices
EXTERN GDT64.Pointer

GLOBAL AbortBootServicesIntx86
GLOBAL AbortBootServicesIntAMD64

 ; Provides interface to AbortBootServices by switching from Paging (32-bit) to no-Paging mode.
AbortBootServicesIntx86:
    ; Clear the PG bit.
    mov eax, cr0
    and eax, ~(1 << 31) & 0xFFFFFFFF
    mov cr0, eax

    call [OldAbortBootServices]

    ; Enable paging again.
    or eax, 0x80000000
    mov cr0, eax

    ; And return.
    ret

 ; Provides interface to AbortBootServices by switching from Paging (64-bit) to no-Paging mode.
AbortBootServicesIntAMD64:
    ; lea rax, [rel $]
    ; push qword 0x18
    ; push qword [rax + Address]
    ; retq

    dw 0x8d48, 0xf905, 0xffff, 0x6aff, 0xff18, 0x11b0, 0x0000, 0x4800
    db 0xcb 

; The address of where to jump to (Bits32) for the far jump.
Address:
    dd .Bits32
    dd 0x00

.Bits32:
    ; Reset all segment registers.
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Clear the PG bit.
    mov eax, cr0
    and eax, ~(1 << 31) & 0xFFFFFFFF
    mov cr0, eax

    call [OldAbortBootServices]

    ; Enable long mode bit in EFER MSR.
    mov ecx, 0xC0000080
    rdmsr
    or eax, (1 << 8)
    wrmsr

    ; Enable paging again.
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    
    lgdt [GDT64.Pointer]
    jmp 0x08:.Bits64

; Switched to 64 bit here.
.Bits64:
    dw 0xB866, 0x0010, 0xD88E, 0xC08E, 0xE08E, 0xE88E
    db 0xC3