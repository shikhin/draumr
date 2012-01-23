 ; Functions to detect and initialize the FPU.
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

SECTION .data

; The no fpu error message.
ErrorFPU: db "ERROR: Unable to find a FPU in the system, required for boot.", 0

SECTION .text

EXTERN AbortBoot
GLOBAL FPUInit

 ; Initializes the FPU, and aborts if it fails to detect/init one.
 ; 
 ; Returns:
 ;     Boot -> aborted if unable to find a FPU.
FPUInit:
    pushad
    
; Try my luck with CPUID, to see if I can detect the FPU using it.
.CPUID:
    pushfd                           ; Get the flags, and pop it into EAX.
   
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
    push ErrorFPU
    jmp AbortBoot

; Store the status word here.
.Status: 
    dw 0x55AA
