; Contains functions responsible for building the memory map.
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

; Tries to build a advanced, sophisticated, sorted, memory map.
MMapBuild:
    pushad

.Prepare:
    xchg bx, bx
    mov di, MMap
    
.BDA:    
    ; Set the BDA entry as 0x400->0x500 of type 4, unusable RAM.
    mov dword [di + 0], 0x400
    mov dword [di + 8], 0x100
    mov dword [di + 16], 4
    mov dword [di + 20], 1
    add di, 24

.Video:
    ; Set the video mapped memory, and other portions as 0xA0000->0xFFFFF of type 4, unusable RAM.
    mov dword [di + 0], 0xA0000
    mov dword [di + 8], 0x60000
    mov dword [di + 16], 4
    mov dword [di + 20], 1
    add di, 24

.BootCode:
    ; Set the boot code, as 0x7C00->End of our file, of type 2, Boot Code.
    mov dword [di + 0], 0x7C00
    mov ecx, [0x9000 + 10]            ; Offset 10 contains the end of the file.
    sub ecx, 0x7C00                   ; Get the length.
    
    ; Round it to the nearest page boundary.
    add ecx, 0xFFF
    and ecx, ~0xFFF
    
    mov dword [di + 8], ecx
    mov dword [di + 16], 2
    mov dword [di + 20], 1
    add di, 24

.E820:
    call E820
    jnc .Clean                        ; If were able to build using E820, then clean the map.
    clc                               ; Clear the carry flag.

.Clean:
    mov esi, MMap
    call MMapSort

.Return:
    popad
    ret

; Tries to use EAX=0x0000E820 method of generating a memory map
; @di             Should point to the memory map area.
;     @rc
;                 Returns with carry set if failed.
E820:
    pushad

    xor ebx, ebx                      ; EBX must be 0 to start.
    xor bp, bp                        ; Store the number of entries in EBP for the moment.
    xor ecx, ecx                      ; The size of the entry is returned in CL - zero out the other bits.
    
    mov edx, 0x0534D4150              ; Expects "SMAP" in edx
    mov eax, 0xE820

    mov dword [es:di + 20], 1         ; If it returns a ACPI 2.0 entry, we should set the "don't ignore this entry" flag in the extended flag attributes.
    mov ecx, 24
    int 0x15

    jc .Error                         ; Error occured while trying to obtain memory map.
    
    mov edx, 0x0534D4150             
    cmp eax, edx                      ; If EAX isn't "SMAP" (signature), then error.
    jne .Error

    test ebx, ebx                     ; EBX specifies the number of entries remaining. If zero, there was only one entry. Error!
    jz .Error

    jmp .Verify                       ; Let's verify the first entry now - once we've verified the call itself.

; Loop till all the entries aren't done.
.Loop:
    mov eax, 0xE820		
    mov dword [es:di + 20], 1         ; See above.
    mov ecx, 24		

    int 0x15
    jc .Finished                      ; All entries done? Finish.
    mov edx, 0x0534D4150

; Verify the entry.
.Verify:
    jcxz .Next                        ; If the entry is zero bytes, skip.
    
    cmp cl, 20
    jbe .Verify2
    
    test byte [es:di + 20], 1         ; If the first bit isn't set in the flags field, then ACPI wants us to ignore this.
    jz .Next

; Verify the entry - stage 2.
.Verify2:
    mov ecx, [es:di + 8]
    or ecx, [es:di + 12]              ; If the length of the entry is zero, than ignore the entry.
    jz .Next

.Reserved:
    cmp dword [es:di + 16], 2
    jne .NVS
    mov dword [es:di + 16], 4
    jmp .Cont

.NVS:
    cmp dword [es:di + 16], 4
    jne .Bad
    mov dword [es:di + 16], 5
    jmp .Cont

.Bad:
    cmp dword [es:di + 16], 5
    jne .Unknown
    mov dword [es:di + 16], 6
    jmp .Cont

.Unknown:
    cmp dword [es:di + 16], 6
    jbe .Cont
    mov dword [es:di + 16], 4

.Cont:
    inc bp                            ; Everything verified? Move to next entry.
    add di, 24

.Next:
    test ebx, ebx                     ; If EBX isn't zero, then loop.
    jnz .Loop

.Finished:
    mov [MMapHeader.Entries], bp      ; Store the entry count
    popad
    clc                               ; Everything was successful.
    ret

.Error:
    popad
    stc
    ret    

; Swaps the entry with its next entry.
; @esi            The first entry.
; @esi + 24       The second entry.
MMapSwap:
    pushad
    
    ; Get the first entry in the registers, and push them.
    mov eax, [esi]
    mov ebx, [esi + 4]
    mov ecx, [esi + 8]
    mov edx, [esi + 12]
    mov ebp, [esi + 16]
    mov edi, [esi + 20]
    pushad

    ; Then get the second entry in the registers, and put them in place of the first.
    mov eax, [esi + 24]
    mov ebx, [esi + 28]
    mov ecx, [esi + 32]
    mov edx, [esi + 36]
    mov ebp, [esi + 40]
    mov edi, [esi + 44]

    mov [esi], eax
    mov [esi + 4], ebx
    mov [esi + 8], ecx
    mov [esi + 12], edx
    mov [esi + 16], ebp
    mov [esi + 20], edi

    ; Then, pop the first entry in the registers, and put them in place of the second.
    popad
    mov [esi + 24], eax
    mov [esi + 28], ebx
    mov [esi + 32], ecx
    mov [esi + 36], edx
    mov [esi + 40], ebp
    mov [esi + 44], edi


.Return:
    popad
    ret


; Sorts the memory map using Bubble sort.
; @esi            Points to the memory map.
MMapSort:
    pushad
    mov bp, [MMapHeader.Entries]      ; The number of entries in the memory map.

.Loop:
    mov word [Finalized], 0
    mov ecx, 1                        ; Now Finalized is the count after which all elements have been sorted.
                                      ; Start the inner loop, where we check each entry for swapping.
.Loop2:
    mov eax, [si]          
    cmp eax, [si + 24]

    jb .FLoop2
    ja .Swap

    mov eax, [si + 4]
    cmp eax, [si + 28]

    jb .FLoop2

.Swap:                                ; If swapping is needed, swap [si] with [si + 24]
    call MMapSwap
    mov word [Finalized], cx          ; Now, if no further swaps are done, then all the elements after this are finalized.

.FLoop2:
    add si, 24
    inc cx                            ; Go the the next count, and entry, till all - 1 have been done.
    cmp cx, bp
    jb .Loop2

.FLoop:
    mov bp, [Finalized]               ; Now, Finalized would contain till where we have finalized - after which we don't need to sort.
    cmp word [Finalized], 0           ; If we've finalized till 0, excellent. No need to loop again.
    jne .Loop 

.Return:
    popad
    ret

SECTION .data

Finalized:        dw 0                ; The finalized variable.
; The Memory Map header.
MMapHeader:
    .Entries      dw 0                ; The number of entries in the memory map.
    .Address      dw MMap             ; The starting address of the memory map.

SECTION .bss

ALIGN 4
; Reserve enough for 512 24-byte entires.
MMap:
    resb 0x3000
