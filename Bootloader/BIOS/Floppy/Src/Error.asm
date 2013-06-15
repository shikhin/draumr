 ; Error handling functions.
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

; The error message in two parts.
errorMessage0:
    db "Boot error: ", 0x00

errorMessage1:
    db ". Aborting boot.", 0x00

 ; Function for aborting boot.
 ;     EAX -> should contain the error code.
Error_abortBoot:
    ; Disable interrupts till we're done messing with the PIT.
    cli
    
    ; Print the beginning of the error message.
    mov si, errorMessage0
    call Display_outputString

    ; The error code.
    call Display_outputHex

    ; End of error message.
    mov si, errorMessage1
    call Display_outputString

; Beep at note A, 5th octave, 880Hz, credits to contrapunctus from #music at freenode.
;   reloadValue = 1193180/frequency; (1193180 -> PIT oscillator)
;               = 1193180/880 = 0x54C;
.beep:
    ; Write to command register (0x43).
    ;   10b  -> channel 2.
    ;   11b  -> access mode lobyte/hibyte.
    ;   011b -> mode 3, square wave generator.
    ;   0b   -> 16-bit binary mode.
    mov al, 10110110b          
    out 0x43, al

    ; I/O delay.
    jmp $+2
    jmp $+2

    ; Write to channel 2, data port (0x42).
    ; Lower 8-bits.
    mov al, 0x4C
    out 0x42, al            

    ; I/O delay.
    jmp $+2
    jmp $+2

    ; Upper 8-bits.
    mov al, 0x05
    out 0x42, al        
    
    ; Port 0x61 controls gate input for PC speaker.
    ; Bit 0 enables timer input to PC speaker; bit 1 enables speaker.
    in al, 0x61                  
    or al, 00000011b
    out 0x61, al                   
   
    ; Can enable interrupts.
    sti

.halt:
    hlt
    jmp .halt