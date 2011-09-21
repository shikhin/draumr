; The functions to switch to paging mode.
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
; 51 Franklin Street, Fifth Floor, Boston, MA 0211

; We are in 32-bits mode here.
BITS 32

; Reserve space for the page directory and page table.

SECTION .text

; Enable paging here - it's awesome. ;-)
EnablePaging:
    pushad

    ; Clear out the PD and PT area.
    mov edi, 0xB000
    xor eax, eax
    mov ecx, 2048

    rep stosd
    
    ; Point the Page Directory's first entry to the page table.
    mov dword [0xB000], 0xC000
    or dword [0xB000], 3
   
    ; Set up 3 - Present, Read/Write, Blah.
    mov eax, 3
    mov esi, 0xC000

.Loop:
    ; Point the page table entry to EAX - and increase EAX and ESI.
    mov [esi], eax
    add eax, 4096    
    add esi, 4

    ; if EAX has not reached till 0x100000, loop.
    cmp eax, 0x100000
    jb .Loop
    
    ; Load the address of the page directory into CR3.
    mov eax, 0xB000
    mov cr3, eax
    
    ; And enable paging.
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

.Return:
    popad
    ret

; Get back to 16-bits mode here.
BITS 16