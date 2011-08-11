; Functions for aborting boot (using both advanced, and non advanced methods).
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


; Function for aborting boot.
; es:si           Should contain the error message to print. 
; ax              Should contain the error code for the "type of beep":
;                 a) If ax is zero, only one long beep.
; TODO: Need to add PXE errors.
AbortBoot:
    cli
    
    ; Print error message on to screen.
    call Print

    ; If AX is zero, use basic method.
    test ax, ax
    jz .Base

    ; Test which beep to produce.
    jmp .Base

.Base: 
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
   
    jmp .Exit

.Exit:
    sti
.Halt:
    hlt
    jmp .Halt


