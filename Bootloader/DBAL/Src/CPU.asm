 ; Contains function for checking if the CPU is supported or not.
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

BITS 32
CPU 486

EXTERN AbortBoot
GLOBAL CPUCheck

; End (of) line.
%define EL           '\n'

; Abort boot if the CPU isn't P5 or above.
ErrorCPUUnsupportedMsg:
    db "CPU doesn't fit in the requirements bracket - P5 or above.", EL, 0

; The no fpu error message.
ErrorFPUNotPresentMsg: 
    db "FPU required for boot operations not present.", EL, 0

 ; Checks if the CPU is supported or not.
 ;
 ; Returns:
 ;     Boot -> boot is halted if CPU not supported.
CPUCheck:
    pushad

    ; Check if the CPU is 486 and/or above or not.
    ; The "sort-of reliable" way to test this is to toggle the AC flag in the EFLAGS.
    pushfd                            ; Get the EFLAGS.
    pop eax                           ; Into the EAX register.
    mov ebx, eax                      ; Backup EAX into EBX.

    xor eax, (1 << 18)                ; Flip the AC flag.
    push eax                          ; Push back the EAX register.
    popfd                             ; And pop back the EFLAGS.

    pushfd                            ; Push the EFLAGS back again, and save into EAX register.
    pop eax

    xor eax, ebx                      ; Set the 'changed bits' to 1.
    test eax, (1 << 18)               ; The 18th bit should now be 1.
    jz .ErrorCPUUnsupported           ; If 18th bit isn't 1, then it isn't a 486.

    ; CPUID should be supported, so check it.
    pushfd                            ; Get the flags, and pop it into EAX.
   
    pop eax
    mov ecx, eax                      ; Save the original flags. 

    xor eax, 0x200000                 ; Flip the bit indicating presence of CPUID.
    push eax                          ; Get the new flags by this. 
    popfd

    pushfd                            ; Get the flags back again.
    pop eax
    xor eax, ecx                      ; And mask the changed bits.

    push ecx                          ; Restore the original flags.
    popfd 
  
    test eax, (1 << 21)               ; Test for the 21st bit.
    ; If it isn't set, then no CPUID.
    jz .ErrorCPUUnsupported

CPU 586
    ; Get the maximum supported basic level.
    xor eax, eax
    cpuid

    ; If standard level 1 isn't supported, ERROR!
    cmp eax, 0x00000001
    jb .ErrorFPUNotPresent

    ; If standard level 3 is the maximum supported, "Limit CPUID MaxVal" might be set.
    cmp eax, 0x00000003
    jne .Cont

    ; Clear bit 22 in MSR 0x1A0 - the "Limit CPUID MaxVal" bit in "IA32_MISC_ENABLE".
    mov ecx, 0x1A0
    rdmsr

    ; Clear bit 22.
    and eax, ~(1 << 22)
    wrmsr

.Cont:    
    ; Check for FPU.
    ; Increase EAX to 0000_0001h.
    xor eax, eax
    inc al
    cpuid

    ; EDX has the feature flags.
    ; Bit 0 specifies the FPU presence.
    test edx, (1 << 0)
    jz .ErrorFPUNotPresent

.Return:
    popad
    ret
    
.ErrorCPUUnsupported:
    push ErrorCPUUnsupportedMsg
    call AbortBoot

.ErrorFPUNotPresent:
    push ErrorFPUNotPresentMsg
    call AbortBoot
