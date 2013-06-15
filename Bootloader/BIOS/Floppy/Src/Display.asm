 ; Functions to display text/numbers.
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

 ; Clears the display to get rid of BIOS messages, and disables the hardware cursor.
Display_init:
    pushad
    push es
   
    ; Set es to 0xB800.
    mov ax, 0xB800
    mov es, ax

    ; Set the cursor to 00,00 so that all future BIOS calls print at the right place.
    ; AH = 0x02, int 0x10, to set cursor position.
    ; BH = 0x00, page number.
    ; DH = row; DL = column.
    xor bx, bx
    xor dx, dx
    mov ah, 0x02
    int 0x10

    ; Hide the cursor.
    ; AH = 0x01, int 0x10, cursor shape.
    ; CL = bottom scan line containing cursor; CH = 0x10 = invisible cursor.
    ; AL needs to be equal to 0x03 (current mode) to prevent bugs on some systems.
    mov cx, 0x1000
    mov ah, 0x01
    int 0x10

    ; Clear screen to blue background, white foreground, spaces.
    xor di, di
    mov ecx, 1000
    mov eax, 0x0F200F20
    rep stosd

    pop es
    popad
    ret

 ; Outputs a string using the BIOS.
 ;     ES:SI -> should contain the address of the null terminated string.
Display_outputString:
    push si
    push ax

    ; Some BIOSs' may destroy BP if display is getting scrolled.
    push bp

.loop:
    ; Load the value at [es:si] in al.
    lodsb

    ; Stop if it's the null terminator.
    test al, al
    jz .printDone

    ; Output character; AH = 0x0E; AL = character; BH = page number = 0x00;    
    mov ah, 0x0E
    xor bh, bh
    int 0x10
    jmp .loop

.printDone:
    pop bp
    pop ax
    pop si
    ret

 ; Outputs a hexadecimal value.
 ;     EAX -> contains the value to output.
Display_outputHex:
    ret 