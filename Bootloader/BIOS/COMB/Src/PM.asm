 ; Functions to enter (and exit) Protected Mode.
 ;
 ; Copyright (c) 2013, Shikhin Sethi
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

 ; Performs a switch to protected mode - making sure to save all registers (except segment one - of course).
 ;     EBX -> the return address here.
PMSwitch:
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
RMSwitch:
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