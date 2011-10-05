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

    or byte [BIT.VideoFlags], VBE_PRESENT   ; So VBE is present, and we know that much.
    mov dword [BIT.VideoInfo], VideoInfo    ; Get it's address.

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

; Get's the information of all video modes from VBE.
; @eax            The 32-bit address (below 1MiB) of a buffer long enough to contain all entries.
;                 Should be page aligned.
VBEGetModeInfo:
    pushad
    
    ; Get the offset into DI.
    xor edi, edi 
    mov di, ax

    ; And then, clear the offset part from EAX.
    sub eax, edi

    ; Clear out EDX. 
    xor edx, edx

    ; And then, divide EDX:EAX by 0x10 - and get the quotient in EAX.
    mov ecx, 0x10
    div ecx

    ; And finally, get the segment in ES.
    mov es, ax

    mov eax, [ControllerInfo + 0x0E]  ; Get the segment offset pair into EAX.
    mov ebx, eax                      ; Get the segment offset pair into EBX.
    shr eax, 16                       ; And now, get the segment in EAX.
  
    ; Now, fs:bx points to the VESA Video mode list.
    mov fs, ax 
    
; Gather video mode information for all video modes.
.Loop:
    xchg bx, bx
    ; 0xFFFF signifies the end of the list - if yes, go to the Finish part.
    cmp word [fs:bx], 0xFFFF
    je .Return

    add di, 2

    ; Try to get information for this video mode.
    mov ax, 0x4F01
    mov cx, [fs:bx]
    int 0x10

    sub di, 2

    test ah, ah
    jnz .NextVideoModeList            ; If AH isn't zero, then something failed. Go to next video mode entry.

    ; Store whatever the mode identifier is in the first word of [es:di].
    mov ax, [fs:bx]
    mov [es:di], ax

.NextOutputBuffer:
    ; Move to the next entry in the output buffer.
    cmp di, (0xFFFF - (256 + 4 + 2)) + 1
    jb .ContOutputBuffer

    ; If we are about to cross (0xFFFF - (256 + 4 + 2)) + 1, then next segment.
    mov ax, es
    inc ax
    mov es, ax

.ContOutputBuffer:
    add di, 256 + 4 + 2

.NextVideoModeList:
    ; Move to the next entry in the video mode list.
    cmp bx, 0xFFFE
    jb .Cont
   
    ; If we are about to cross 0xFFE, move on to the next segment.
    mov ax, fs
    inc ax
    mov fs, ax

.Cont:
    add bx, 2
    jmp .Loop

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