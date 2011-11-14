; Functions to print to the screen and initialize it.
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


BITS 16


; Clears the screen to get rid of BIOS' messages, and disables the hardware cursor.
;     @es         Should contain the segment containing Video Memory.
InitScreen:
    pushad                            ; Push all general purpose registers to save them.

    ; Set to mode 0x03, or 80*25 text mode.
    mov ah, 0x00
    mov al, 0x03
   
    ; SWITCH! 
    int 0x10

    xor ebx, ebx
    xor edx, edx
    mov eax, 0x200
    int 0x10                          ; Set the position of the hardware cursor to 00, 00 so that later BIOS calls
                                      ; print at the right place.
    mov ecx, 0x2600
    mov eax, 0x100
    int 0x10                          ; Hide the hardware cursor.

    xor di, di
    mov ecx, 1000                     ; Since we are clearing DWORDs over here, we put the count as Count/4.
    mov eax, 0x1F201F20               ; Set the value to set the screen to: Blue background, white foreground, blank spaces.
    rep stosd                         ; Clear the entire screen. 
    
    popad                             ; Pop all general registers back to save them.
    ret


; Prints a message on the screen using the BIOS.
;     @si         Should contain the address of the null terminated string.
Print:
    pushad

.PrintLoop:
    lodsb                             ; Load the value at [@es:@si] in @al.
    test al, al                       ; If AL is the terminator character, stop printing.

    je .PrintDone                  	
    mov ah, 0x0E	
    int 0x10
    jmp .PrintLoop                    ; Loop till the null character not found.
	
.PrintDone:
    popad                             ; Pop all general purpose registers to save them.
    ret	

