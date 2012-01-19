 ; Functions to print to the screen and initialize it.
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

 ; Clears the screen to get rid of BIOS' messages, and disables the hardware cursor.
InitScreen:
    pushad                            ; Push all general purpose registers to save them.
    push es                           ; And es.
   
    ; Set es to 0xB800.
    mov ax, 0xB800
    mov es, ax
    
    ; Set to mode 0x03, or 80*25 text mode.
    mov ax, 0x03
   
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
    
    pop es                            ; Restore es
    popad                             ; Pop all general registers back to save them.
    ret

 ; Prints a message on the screen using the BIOS.
 ;     SI -> the address of the null terminated string.
Print:
    ; Save some registers.
    push si
    push ax

.PrintLoop:
    lodsb                             ; Load the value at [@es:@si] in @al.
    test al, al                       ; If AL is the terminator character, stop printing.

    je .PrintDone                  	
    mov ah, 0x0E	
    int 0x10
    jmp .PrintLoop                    ; Loop till the null character not found.
	
.PrintDone:
    ; Restore them.
    pop ax
    pop si
    ret