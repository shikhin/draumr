 ; A20 enabling code for BIOS common file.
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

 ; Tries to enable A20.
 ; 
 ; Returns:            
 ;     BIT.HrdwreFlags -> Sets A20_DISABLED flag if unable to enable A20.
A20Enable:
    pushad

    call A20Check
    jc .Return                        ; If A20 is already enabled, then return. 

; Try the BIOS here to enable A20.
.BIOS:
    mov ax, 0x2401
    int 0x15

    call A20Check
    jc .Return

; Try the keyboard controller.
.Keyboard:

; The 8042 controller's "input buffer" needs to be clear - so we often check for it.
.D1:
    jmp $ + 2
    jmp $ + 2

    in al, 0x64
    test al, 2
    jnz .D1

    mov al, 0xD1                      ; Send the write command.
    out 0x64, al
    
.D2:
    jmp $ + 2
    jmp $ + 2

    in al, 0x64
    test al, 2
    jnz .D2

    mov al, 0xDf                      ; And then, the A20 on command.
    out 0x60, al

.D3:
    jmp $ + 2
    jmp $ + 2

    in al, 0x64
    test al, 2
    jnz .D3

    call A20Check
    jc .Return

.FastA20:
    in al, 0x92                       ; Get the value from 0x92 port. 
    test al, 02                       ; If the Fast A20 bit is already set, then, Fast A20 can't be relied upon.
    jnz .Disabled

    or al, 02                         ; Enable Fast A20.
    out 0x92, al      

    call A20Check
    jc .Return

.Disabled:
    or byte [BIT.HrdwreFlags], A20_DISABLED

.Return:
    clc
    popad
    ret

 ; Checks the status of A20, and returns what it is.
 ;
 ; Returns:
 ;     Carry -> set if A20 is ENABLED, else DISABLED.
A20Check:
    pushad
    push ds
    push es

    xor ax, ax                        ; Zero out AX, and move it into ES.
    mov es, ax                     

    not ax                            ; And then, move 0xFFFF into DS.
    mov ds, ax

    mov di, 0x510                     ; DS:DI points to 0x100500.
    mov si, 0x500                     ; While ES:SI points to 0x500.

    mov byte [es:si], 0x00            ; 0x500 now has 0x00 stored in it.

    ; Here comes the tricky bit. If A20 has been enabled, then DS:DI would point to 0x100500.
    ; Else, DS:DI would point to 0x00500, so a move over there would overwrite ES:SI.

    mov byte [ds:di], 0xFF          

    mov eax, [es:si]
    cmp eax, 0xFF                     ; Now, if ES:SI is 0xFF, then A20 isn't enabled.
    je .Return

    stc

.Return:
    pop es
    pop ds
    popad
    ret