; Functions to detect and initialize the FPU.
;
; Copyright (c) 2011 Shikhin Sethi
; 
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation;  either version 3 of the License, or
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

BITS 32

SECTION .data

; The no fpu error message.
ErrorNoFPU: db "ERROR: Unable to find a FPU in the system, required for boot.", 0

SECTION .text

EXTERN AbortBoot
GLOBAL FPUInit
; Tries to detect a FPU, and initialize it if it finds one.
;     rc
;                                     Aborts boot if unable to find a FPU.
FPUInit:
    pushad
    
; Try my luck with CPUID, to see if I can detect the FPU using it.
.CPUID:
    pushfd                           ; Get the flags, and pop it into @eax.
   
    pop eax
    mov ecx, eax                     ; Save the original flags. 

    xor eax, 0x200000                ; Flip the bit indicating presence of CPUID.
    push eax                         ; Get the new flags by this. 
    popfd

    pushfd                           ; Get the flags back again.
    pop eax
    xor eax, ecx                     ; And mask the changed bits.

    push ecx                         ; Restore the original flags.
    popfd 
  
    test eax, (1 << 21)              ; Test for the 21st bit.
    ; If it isn't set, then no CPUID.
    jz .Probe

    ; Do CPUID with EAX = 0000_0000h, to find the maximum supported standard level.
    xor eax, eax
    cpuid

    ; If EAX is zero, which means that we can't use standard level 0000_0001h.
    test eax, eax
    jz .Probe

    ; Increase EAX to 0000_0001h.
    xor eax, eax
    inc al
    cpuid

    ; EDX has the feature flags.
    ; Bit 0 specifies the FPU presence.
    test edx, (1 << 0)
    jz .NoFPU

    mov eax, cr0
    and eax, ~((1 << 2) | (1 << 3))
    mov cr0, eax

    fninit

    jmp .Return

; Probe for a FPU. The best way, I found out, by the osdev wiki, is to initialize the FPU
; without any waits, then attempt to store the status word. If the status word isn't zero
; then the FPU isn't present.
.Probe:
    ; Get CR0 in eax and save it.
    mov eax, cr0

    and eax, ~((1 << 2) | (1 << 3))
    mov cr0, eax                      ; Clear the Task Switch and Emulate FPU flags - so that the fpu works properly.

    ; Initialize the FPU WITHOUT any wait, and then have it store the status word, again without any wait.
    fninit
    fnstsw [.Status]

    ; If the status wasn't 0, which it should be after a fninit, then abort!
    cmp word [.Status], 0
    jne .NoFPU
 
.Return:
    ; Do some final changes.
    mov eax, cr0
    ; Use Native Exceptions, and have FWAIT cause a fpu state update 
    or eax, (1 << 5) | (1 << 1)
    mov cr0, eax

    popad
    ret

.NoFPU:
    push ErrorNoFPU
    call AbortBoot

; Store the status word here.
.Status: 
    dw 0x55AA
