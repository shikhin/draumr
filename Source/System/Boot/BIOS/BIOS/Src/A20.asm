; A20 enabling code for BIOS common file.
;
; Copyright (c) 2011 Shikhin Sethi
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 3 of the License, or
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


SECTION .text

; Tries to enable A20.
; @rc            
;                 If due to any reasons we were unable to enable A20, it sets the A20_DISABLED flag in BIT.HrdwreFlags
EnableA20:
    pushad

    call CheckA20
    jc .Return                        ; If A20 is already enabled, then return. 

; Try the BIOS here to enable A20.
.BIOS:
    mov ax, 0x2401
    int 0x15

    call CheckA20
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

    call CheckA20
    jc .Return

.FastA20:
    in al, 0x92                       ; Get the value from 0x92 port. 
    test al, 02                       ; If the Fast A20 bit is already set, then, Fast A20 can't be relied upon.
    jnz .Disabled

    or al, 02                         ; Enable Fast A20.
    out 0x92, al      

    call CheckA20
    jc .Return

.Disabled:
    or byte [BIT.HrdwreFlags], A20_DISABLED

.Return:
    clc
    popad
    ret

; Checks the status of A20, and returns what it is.
;     @rc
;                 Sets the carry flag, if A20 is ENABLED, else DISABLED.
CheckA20:
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
