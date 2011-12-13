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

%define BD_CD      0
%define BD_FLOPPY  1
%define BD_PXE     2

SECTION .text

; Tries to build a advanced, sophisticated, sorted, memory map.
MMapBuild:
    pushad

    mov dword [BIT.MMap], MMapHeader

.Prepare:
    mov di, MMap
    xor bp, bp                        ; The number of entries in the MMap.

; TODO: Check if were booted from PXE, and if yes - mark the portion from 0x80000 to 0xC0000 as reclaimable.
    cmp byte [BIT.BDFlags], BD_PXE
    jne .BDA
    
.PXE:
    ; Set the PXE entry as 0x80000->0xC0000 of type 2, boot code (so that it can be reclaimed later).
    mov dword [di + 0], 0x80000
    mov dword [di + 8], 0x40000
    mov dword [di + 16], 2
    mov dword [di + 20], 1
    add di, 24
    inc bp

.BDA:    
    ; Set the BDA entry as 0x400->0x500 of type 4, unusable RAM.
    mov dword [di + 0], 0x400
    mov dword [di + 8], 0x100
    mov dword [di + 16], 4
    mov dword [di + 20], 1
    add di, 24
    inc bp

.Video:
    ; Set the legacy BIOS ROM area, and other portions as 0xC0000->0xFFFFF of type 4, unusable RAM.
    mov dword [di + 0], 0xC0000
    mov dword [di + 8], 0x40000
    mov dword [di + 16], 4
    mov dword [di + 20], 1
    add di, 24
    inc bp

    ; And then the real BIOS rom, 0xFE000000->0xFFFFFFFF of type 4, unusable RAM.
    mov dword [di + 0], 0xFE000000
    mov dword [di + 8], 0x2000000 
    mov dword [di + 16], 4
    mov dword [di + 20], 1
    add di, 24
    inc bp

.BootCode:
    ; Set the boot code, as 0x7C00->End of our file, of type 2, Boot Code.
    mov dword [di + 0], 0x7C00
    mov ecx, 0x40000                  ; TODO: Fix later. Let's assume the area till 0x40000 is used by Boot Code.
    sub ecx, 0x7C00                   ; Get the length.
    
    mov dword [di + 8], ecx
    mov dword [di + 16], 2
    mov dword [di + 20], 1
    add di, 24
    inc bp

.E820:
    call E820
    jnc .Clean                        ; If were able to build using E820, then clean the map.
    
    clc                               ; Clear the carry flag.

; Int 0x12 would be the first to use (to get the, um, "base memory data").
.12:
    xor eax, eax                      ; Clear out AX.
    int 0x12                          ; Call Int 0x12 - I assume it always works, which it should - if it doesn't, well, then, I don't think this would happen. :-)
    mov ecx, 1024
    mul ecx                           ; Now we got the length of the base entry in EAX.

    mov dword [di + 8], eax           ; Get the length of the entry.
    mov dword [di + 16], 1
    mov dword [di + 20], 1            ; Now that's free RAM - type 1.
    add di, 24                        ; That was the 'free' RAM area - let's do the 'system' RAM area.
    inc bp

    mov dword [di + 0], eax           ; Now, it starts at EAX - end of previous entry.
    mov ebx, 0xA0000

    cmp eax, 0xA0000                  ; If the free RAM region is around above 0xA0000 - wrap it to 0xA0000 (and the mess would be cleaned later).
    jb .Cont

    mov eax, 0x9FFFF

.Cont:
    sub ebx, eax                      ; EBX now contains the length of this entry.

    mov dword [di + 8], ebx
    mov dword [di + 16], 4
    mov dword [di + 20], 1
    add di, 24                        ; Move to next entry.
    inc bp

    mov dword [di], 0xF00000          ; Make the 0xF00000 - 0x1000000 entry for the ISA memory hole.
    mov dword [di + 8], 0x100000
    mov dword [di + 16], 4
    mov dword [di + 20], 1
    add di, 24
    inc bp
 
; There is a bigger chance that E881 succeeds where E801 fails.
.E881:
    mov eax, 0xE881
    call E8X1                         ; E8X1 simply works on whatever is in EAX.
    jnc .Clean                        ; If carry wasn't set, clean the map.

    clc                               ; Else, clear the carry flag, and try 0xE801.

.E801:
    mov eax, 0xE801                   ; Or try 0xE801 now.
    call E8X1                         
    jnc .Clean        

    clc

; Well, this is really confusing - which one is more reliable? I assume 0x8A is more reliable than 0xDA88.
.8A:
    ; Clear out EDX and EAX - since we'd need the higher word to be clear in any case.
    xor edx, edx
    xor eax, eax
    
    mov ah, 0x8A
    int 0x15
    jc .DA88

    ; Now, that should move the lower word into the higher word.
    shl edx, 16
    ; And put that into EAX.
    or eax, edx

    ; Since it is length in 1KiB block, multiply it by 1024 to get the length in bytes.
    mov ecx, 1024
    mul ecx

    mov dword [di], 0x100000
    mov dword [di + 8], eax
    ; That's free RAM - and a usable entry.
    mov dword [di + 16], 1
    mov dword [di + 20], 1
    add di, 24

    ; Increase the "we have that many entries" count.
    inc bp
    
    ; If the entry we just added spans below the 16MiB mark, probe for more memory at 16MiB.
    add eax, 0x100000
    cmp eax, 0xF00000
    jb .HigherManual

    mov [MMapHeader.Entries], bp

    jmp .Clean

.DA88:
    clc

    ; Clear out ECX and EBX - since we'd need the higher word to be clear in any case.
    xor ecx, ecx
    xor ebx, ebx
    
    mov eax, 0x8A
    int 0x15
    jc .88

    mov eax, ebx
    ; Now, that should move the lower word into the higher word.
    shl ecx, 16
    ; And put that into EAX.
    or eax, ecx

    ; Since it is length in 1KiB block, multiply it by 1024 to get the length in bytes.
    mov ecx, 1024
    mul ecx

    mov dword [di], 0x100000
    mov dword [di + 8], eax
    ; That's free RAM - and a usable entry.
    mov dword [di + 16], 1
    mov dword [di + 20], 1
    add di, 24

    ; Increase the "we have that many entries" count.
    inc bp

    ; If the entry we just added spans below the 16MiB mark, probe for more memory at 16MiB.
    add eax, 0x100000
    cmp eax, 0xF00000
    jb .HigherManual

    mov [MMapHeader.Entries], bp

    jmp .Clean

.88:
    clc

    ; Clear out EAX - since we'd need the higher word to be clear in any case.
    xor eax, eax
    
    mov ah, 0x88
    int 0x15
    jc .CMOS

    ; Since it is length in 1KiB block, multiply it by 1024 to get the length in bytes.
    mov ecx, 1024
    mul ecx

    mov dword [di], 0x100000
    mov dword [di + 8], eax
    ; That's free RAM - and a usable entry.
    mov dword [di + 16], 1
    mov dword [di + 20], 1
    add di, 24

    ; Increase the "we have that many entries" count.
    inc bp

    ; If the entry we just added spans below the 16MiB mark, probe for more memory at 16MiB.
    add eax, 0x100000
    cmp eax, 0xF00000
    jb .HigherManual

    mov [MMapHeader.Entries], bp

    jmp .Clean

.CMOS:
    clc

    xor eax, eax

    ; Tell them we are reading the register (contains higher memory)!
    mov al, 0x31
    out 0x70, al
    in al, 0x71

    ; Shift left eight bits, to get it into AH.
    shl ax, 8

    ; And now the lower memory.
    mov al, 0x30
    out 0x70, al
    in al, 0x71

    ; Since it is length in 1KiB block, multiply it by 1024 to get the length in bytes.
    mov ecx, 1024
    mul ecx

    test eax, eax
    jz .Manual

    mov dword [di], 0x100000
    mov dword [di + 8], eax
    ; That's free RAM - and a usable entry.
    mov dword [di + 16], 1
    mov dword [di + 20], 1
    add di, 24

    ; Increase the "we have that many entries" count.
    inc bp

    ; If the entry we just added spans below the 16MiB mark, probe for more memory at 16MiB.
    add eax, 0x100000
    cmp eax, 0xF00000
    jb .HigherManual

    mov [MMapHeader.Entries], bp

    jmp .Clean

; The manual probing method here.
.Manual:
    clc

    ; Probe for RAM at 1MiB till 15MiB (the ISA memory hole).
    mov esi, 0x100000
    mov edx, 0xE00000
    call Probe

    test ecx, ecx
    jz .ManualEnd                     ; If the probe found no memory - end.

    ; Create an entry for it, and increase the entries count.
    mov dword [di], 0x100000
    mov dword [di + 8], ecx
    mov dword [di + 16], 1            ; It's free RAM - yeah.
    mov dword [di + 20], 1            ; And a usable entry.
    add di, 24
    
    inc bp                            ; Increase the entries count.

; A probe for memory at the 16MiB mark.
.HigherManual:
    ; Let's probe for RAM starting from 0x1000000 (16MiB) to 0xFE000000 (4080MiB).
    mov esi, 0x1000000
    mov edx, 0xFE000000 - 0x1000000
    call Probe

    test ecx, ecx
    jz .ManualEnd                     ; Same as above.

    ; Create an entry for it, and increase the entries count.
    mov dword [di], 0x1000000
    mov dword [di + 8], ecx
    mov dword [di + 16], 1            ; It's free RAM - and a usable entry.
    mov dword [di + 20], 1
    add di, 24

    inc bp                            ; And increase the entries count.

.ManualEnd:
    mov [MMapHeader.Entries], bp

.Clean:
    mov esi, MMap
    call MMapSort

.Return:
    popad
    ret

Dummy:            dd 0

; Probe to see if there's RAM at a certain address (taken and modified from wiki.osdev.org)
; 
; @edx            Maximum number of bytes to test.
; @esi            Starting address of blocks to probe. 
;     @rc
;                 @ecx contains the number of bytes of RAM found 
Probe:  
    mov ebx, .TestStart
    jmp SwitchToPM

BITS 32
.TestStart:
    xor ecx, ecx                      ; The "number of bytes" of RAM found.
    test edx, edx                     ; If @edx is zero, then no need to do anything.
    jz .SwitchToRM

    or esi, 0x00003FFC                ; @esi is the address of the last dword in the first block.
    shr edx, 14                       ; Shift right EDX by 14, to "divide it by 0x4000".
 
.TestBlock:
    mov eax, [esi]                    ; @eax contains the original value at the address.
    mov ebx, eax                      ; @ebx contains the original value at the address too.      
    not eax                           ; Reverse the value in @eax.

    mov [esi], eax                    ; Modify value at address - replace by the reversed value.
    mov [Dummy], ebx                  ; Do a dummy write - and then flush the cache.     
    
    wbinvd                            ; Flush the cache
    
    mov ebp, [esi]                    ; Compare the value at the address with the "reversed value".
    mov [esi], ebx                    ; Restore the original value (even if it's not RAM, in case it's a memory mapped device or something)
    
    cmp ebp, eax                      ; Has the value changed over there - if yes, just quit before we make something more bad.  
    jne .SwitchToRM                   ; Let's return.
 
    ; Increase the bytes found counter, and the "address to test" pointer.
    add ecx, 0x00004000     
    add esi, 0x00004000      

    ; Decrease the blocks remaining counter.
    dec edx              
    test edx, edx
    jnz .TestBlock

.SwitchToRM:   
    mov ebx, .Return
    jmp SwitchToRM

BITS 16
    ; Reload the segment registers, and enable interrupts.
.Return: 
    ret

; Tries to use EAX=0x0000E8X1 method of generating a memory map
; @di             Should point to the memory map area.
; @bp             The number of entries already there.
; @eax            EAX should either contain 0xE881 or 0xE801.
;     @rc     
;                 Returns with carry set if failed.
E8X1:
    pushad

    xor ebx, ebx

    xor ecx, ecx
    xor edx, edx                      ; ECX, EDX would contain the configured memory - if that remains zero, use extended (EAX, EBX)
    
    int 0x15                          ; Try AX=0xE8X1 - whatever it was.
    
    jc .Error                         ; Try E801, let's see - perhaps it works.
    jecxz .Cont                       ; If ECX was zero, correct result is already in EAX, EBX pair.

    mov eax, ecx
    mov ebx, edx                      ; Or put the correct result in EAX, EBX pair.
    
; Now the correct values are in the EAX, EBX pair.
.Cont:
    cmp eax, 0x3C00
    ja .Error                         ; The maximum value should be till the memory hole - if not, then don't trust it.

    mov dword [di + 0], 0x100000      ; This entry starts at 1MiB.
   
    mov ecx, 1024
    mul ecx                           ; Multiply EAX by 1024 - EAX contains 1KiB blocks between 1MiB and 15MiB - so make it bytes.
    mov dword [di + 8], eax
    mov dword [di + 16], 1            ; Free RAM - type 1.
    mov dword [di + 20], 1            ; And 'to be used'.
    add di, 24                        ; Move to next entry.
    inc bp

    mov eax, ebx                      ; Get the extended memory in EAX.
    mov ecx, 64 * 1024                ; And since it's in 64 KiB blocks, multiply it with 64 * 1024.
    mul ecx    
    
    mov dword [di + 0], 0x1000000     ; From 16MiB - above.
    mov dword [di + 8], eax
    mov dword [di + 16], 1            ; Free RAM - type 1.
    mov dword [di + 20], 1            ; And 'to be used'.
    add di, 24                        ; Now let's move to the next entry (if any).
    inc bp                            ; BP now contains the number of entries.

.Finished:
    mov [MMapHeader.Entries], bp      ; Store the entry count
    popad
    clc                               ; Everything was successful.
    ret

.Error:
    popad
    stc
    ret    


; Tries to use EAX=0x0000E820 method of generating a memory map
; @di             Should point to the memory map area.
; @bp             The number of entries already there.
;     @rc
;                 Returns with carry set if failed.
E820:
    pushad

    xor ebx, ebx                      ; EBX must be 0 to start.
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
    push esi
    mov word [Finalized], 0
    mov ecx, 1                        ; Now Finalized is the count after which all elements have been sorted.
                                      ; Start the inner loop, where we check each entry for swapping.
.Loop2:
    mov eax, [si + 4]          
    cmp eax, [si + 28]

    jb .FLoop2
    ja .Swap

    mov eax, [si]
    cmp eax, [si + 24]

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
    pop esi
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
    .Address      dd MMap             ; The starting address of the memory map.

SECTION .bss

ALIGN 4
; Reserve enough for 512 24-byte entires.
MMap:
    resb 0x3000
