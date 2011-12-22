; Contains functions to find the IPS of the machine.
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

SECTION .data

; The current tick (or last read) - for comparision.
Tick:
    dw 0

; We keep this as our counter.
Counter:
    dd 0

SECTION .text

; Finds the Instructions executed in each second by the machine.
; NOTE: Might not be entirely accurate. I just take an average of few
; instructions executed in a loop.
FindIPS:
    pushad
    
; Synchronize the system time, so that we start the loop *immediately after*
; the system time changes - so that we get one entire 54ms duration.
.Sync:
    ; Get system time.
    xor ah, ah
    int 0x1A
    
    mov [Tick], dx
    
; Loop for syncing.
.LoopSync:
    ; Get system time again.
    xor ah, ah
    int 0x1A
    
    ; If it is the same as last time, loop again.
    cmp [Tick], dx
    je .LoopSync
   
    ; Store the new tick.
    mov [Tick], dx

;  Now loop for finding ips.
.BigLoopIPS:
    mov ecx, 100

.LoopIPS:
    ; Note. The instructions following here till the next comment are
    ; just some common instructions I use a lot.
    
    ; If it looks like very random, I intended it to be that way. ;-)
    push ecx
    
    xor ecx, ecx
    not ecx
    sub ecx, 0x7FFFFFFF
    add ecx, [Counter]
    
    mov eax, 0x12123434
    mul ecx
    
    cmp eax, 0x80000000
    jg .C1
    
.C1:
    test edx, edx
    jz .C2
    
.C2:
    mov edi, .Temp
    mov ecx, 10
    
    mov eax, [Counter]
    add eax, 0x5A5A5A5A
    rep stosd
        
    jmp .C3   
     
.Temp:
    times 10 dd 0
    
.C3:
    rep nop
    imul ecx
    
    mov ecx, 10
    push ecx
    
    add ecx, 0x0A0A0A0A
    idiv ecx
    
    or eax, 0xF000000F
    and eax, ~0x7F
    
    shr eax, 0x4
    xchg al, bl
    add eax, 0x50050050
    shl eax, 4
    
    dec eax
   
    call .C4
    jmp .C5
   
.C4: 
    ret
   
.C5:     
    pop ecx
    
.C6:
    rep nop
    loop .C6

    pop ecx
    dec ecx
    jnz .LoopIPS
    
    ; Get system time again.
    xor ah, ah
    int 0x1A
    
    ; If it is the same as last time, loop again.
    cmp [Tick], dx
    jne .Calculate
   
    ; Increase the counter.
    inc dword [Counter]
    jmp .BigLoopIPS

.Calculate:
    ; Calculate the IPS based on the counter.
    xor edx, edx
    mov eax, [Counter]
    
    ; Multiply by 18 - we loop for one tick - in one second, there are 18.2 ticks.
    mov ecx, 18
    mul ecx

    ; Multiply by 75 * 100 - the number of instructions - the number of times it is looped - +50
    ; for the big loop.
    mov ecx, (75 * 100) + 50
    mul ecx

    ; Store the IPS.
    mov [BIT.IPS + 4], edx
    mov [BIT.IPS], eax

.Return:
    popad
    ret
