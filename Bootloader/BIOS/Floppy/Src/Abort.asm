 ; Functions for aborting boot (using both advanced, and non advanced methods).
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

SECTION .base

 ; Function for aborting boot.
 ;     ES:SI -> should contain the error message to print. 
AbortBoot:
    cli
    
    ; Save SI.
    push si

    ; Print the Error (slogan) Message.
    mov si, ErrorMsg
    call Print

    ; Restore SI.
    pop si

    ; Print error message on to screen.
    call Print

.Beep: 
    mov al, 10110110b          

    out 0x43, al 

    ; Create I/O delay for old machines.
    jmp $+2
    jmp $+2

    mov al, 0xD1                      ; Send lower 16-bits of count for frequency to play.            
    out 0x42, al            

    ; Create I/O delay for old machines.
    jmp $+2
    jmp $+2

    mov al, 0x11                      ; Send higher 16-bits of count for frequency to play.
    out 0x42, al        
    
    in al, 0x61                  
    or al, 00000011b                  ; Set the Speaker enable (and other required) bit.
    out 0x61, al                      ; SPEAK.                   
   
    sti

.Halt:
    hlt
    jmp .Halt