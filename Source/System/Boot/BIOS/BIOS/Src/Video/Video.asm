; Common definitions to get the Video Mode information list.
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

; TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
; TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT   
; TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT   
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;              TTTT
;
;
; THAT WAS MEANT TO BE A BIG BIG TODO, BUT WHO HAS TO TIME TO DO "ODO"?

SECTION .text

; Builds the Controller information - and the required things to get Video Mode Information List.
VideoInfoBuild:
    pushad

    ; Try getting VBE2 information - if that fails, I'll try with VGA.
    mov dword [ControllerInfo], "VBE2"
    
    mov di, ControllerInfo            ; The output buffer to be moved in DI (the address).
    mov ax, 0x4F00
    int 0x10
    
    ; Here AX would contain the return status, telling about whether we failed or not.
    cmp ax, 0x004F
    jnz .Return                       ; TODO: See whether we can go with VGA or anything else here.

    mov eax, [ControllerInfo + 0x0E]  ; Get the segment offset pair into EAX.
    mov ebx, eax                      ; Get the segment offset pair into EBX.
    shr eax, 16                       ; And now, get the segment in EAX.
  
    ; Now, es:ab points to the VESA Video mode list.
    mov es, ax 
    xor ecx, ecx
    
; Let's attempt to find the number of Video Modes.
.Loop:
    ; 0xFFFF signifies the end of the list - if yes, go to the Finish part.
    cmp word [es:bx], 0xFFFF
    je .LoopF

    ; Else, increase the count and lopp till Loop.
    inc ecx

    ; Move to the next entry.
    cmp bx, 0xFFFE
    jb .Cont
   
    ; If we are about to cross 0xFFE, move on to the next segment.
    mov ax, es
    inc ax
    mov es, ax

.Cont:
    add bx, 2
    jmp .Loop
    
; The Finish Loop part.
.LoopF:
    ; If ECX is zero, then this is just a dummy VBE installation.
    test ecx, ecx
    jz .Return                        ; TODO: See whether we can go with VGA or anything else here.

    mov [VideoInfo.Entries], ecx      ; Store the number of video mode entries into VideoInfo.Entries
  
    mov cx, [ControllerInfo + 0x04]   ; Get the version number - if less than 2.0, no accelerated.
    cmp cx, 0x0200 
    jb .Return

    mov ecx, [ControllerInfo + 0x0A]  ; Get the capabilities flag into ECX.
    test ecx, (1 << 3)                ; Test whether bit three is set or not.
    jz .Return                        ; If not set, accelerated modes not supported.

    mov eax, [ControllerInfo + 0x24]  ; Get the segment offset pair into EAX.
    mov ebx, eax                      ; And in EBX.
    shr eax, 16                       ; And now, get the segment in EAX.

    ; Now, es:ax points to the VESA Accelerated Video mode list.
    mov es, ax 
    xor ecx, ecx
    
; Let's attempt to find the number of Video Modes.
.Loop2:
    ; 0xFFFF signifies the end of the list - if yes, go to the Finish part.
    cmp word [es:bx], 0xFFFF
    je .LoopF2

    ; Else, increase the count and lopp till Loop.
    inc ecx

    ; Move to the next entry.
    cmp bx, 0xFFFE
    jb .Cont2
   
    ; If we are about to cross 0xFFE, move on to the next segment.
    mov ax, es
    inc ax
    mov es, ax

.Cont2:
    add bx, 2
    jmp .Loop2
    
; The Finish Loop.
.LoopF2:
    mov [VideoInfo.EntriesA], ecx     ; And save the number.

.Return:
    popad
    ret


SECTION .bss
; The Video Info structure, directly followed by ControllerInfo structure.
VideoInfo:
    .Entries  resd 1                  ; The number of the entries of video modes.
    .EntriesA resd 1                  ; The number of the entries of accelerated video modes - yeah, who doesn't like them? :-)
; Reserve 512 bytes for the controller info.
ControllerInfo:
    resb 512    