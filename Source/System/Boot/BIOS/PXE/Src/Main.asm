; Entry point for PXE Bootloader Stage 1
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

BITS 16

SECTION .data

%define BD_CD        0
%define BD_FLOPPY    1
%define BD_PXE       2

BIOSError db "ERROR: Error occured while trying to read, open or parse common BIOS File.", 0

; Prepare the PXENV_GET_CACHED_INFO structure.
PXENV_GET_CACHED_INFO:
    .Status       dw 0
    .PacketType   dw 2
    .BufferSize   dw 0
       .BufferOff dw 0                ; A zero over here means it should return the address of it's own buffer.
       .BufferSeg dw 0
    .BufferLimit  dw 0

; Entry point where the PXE hands control.
; @cs:ip          Expects CS:IP to point to the linear address 0x7C00.
; @es:bx          Expects it to point to the PXENV+ structure.
; @ss:sp + 4      Expects it to point to the !PXE structure.
GLOBAL Startup
SECTION .text
Startup:
    jmp 0x0000:Main                   ; PXE promises to load the bootloader at 0x0000:0x7C00 However, no trusting
                                      ; anything - so doing a far jump to reload this value
                                      ; to a standard 0x0000:0xIP.

%include "Source/System/Boot/BIOS/PXE/Src/Screen.asm"
%include "Source/System/Boot/BIOS/PXE/Src/Abort.asm"
%include "Source/System/Boot/BIOS/PXE/Src/PXE/PXE.asm"
%include "Source/System/Boot/BIOS/PXE/Src/PXE/Disk.asm"
%include "Source/System/Boot/Lib/CRC32/CRC32.asm"

Main:
    cli                               ; Stop maskable interrupts till a proper stack is set up.

    call InitScreen                   ; Initialize the entire screen to blue, and disable the hardware cursor.					
    call InitPXE
   
    xor ax, ax                        ; Set all the segment registers to 0x0000.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

.GetCachedInfo:
    mov di, PXENV_GET_CACHED_INFO
    mov bx, GET_CACHED_INFO
    
    call UsePXEAPI                    ; Ask for the cached info.
    
    or ax, [PXENV_GET_CACHED_INFO]    ; Get the status into BX.
    jnz .Error

    movzx esi, word [PXENV_GET_CACHED_INFO.BufferOff]
    add esi, 20                       ; Put the source offset into ESI.
       
    mov edi, SIP                      ; And address of destination in EDI.
    
    mov ax, [PXENV_GET_CACHED_INFO.BufferSeg]
    mov ds, ax                        ; And the segment of source into DS.
    movsd                             ; Save SIP somewhere.
    movsd                             ; And GIP.
 
    xor ax, ax
    mov ds, ax
 
.LoadCommonBIOS:
    xor ax, ax                        ; Open File 0, or common BIOS file.
    
    call OpenFile                     ; Open the File.
    jc .Error

    ; ECX contains size of file we are opening.
    push ecx
    
    mov ecx, 512                      ; Read only 512 bytes.
    mov edi, 0x9000
    call ReadFile                     ; Read the entire file.

; Checks common BIOS file from first sector.
.CheckCBIOSFirstSector:
    cmp dword [0x9000], "BIOS"        ; Check the signature.
    jne .Error2

    cmp dword [0x9008], 0x9000        ; Check whether starting address is 0x9000 or not.
    jne .Error2                       ; If no, abort boot.
        
    mov ecx, [0x9000 + 12]            ; Get the end of file in ECX.
    sub ecx, 0x9000                   ; Subtract 0x9000 from it to get it's size.
    add ecx, 0x7FF
    shr ecx, 11                       ; Here we have the number of sectors of the file (according to the header).
 
    mov edx, [esp]                    ; Get the stored ECX.
    
    add edx, 0x7FF
    shr edx, 11                       ; Here we have the number of sectors of the file (according to the fs).

    cmp ecx, edx
    jne .Error2                       ; If they aren't equal, error.

.LoadRestFile:
    add edi, 512
    pop ecx
    
    cmp ecx, 512
    jbe .Finish

    sub ecx, 512                      ; Read the rest bytes.
    
    call ReadFile                     ; Read the rest of the file.

.Finish:
    call CloseFile                    ; And then close the file.

; Check the rest of the common BIOS file.
.CheckCBIOSRest:
    mov ecx, [0x9000 + 12]            ; Get the end of the file in ECX.
    mov esi, 0x9000 + 28              ; Calculate CRC from above byte 24.
    
    sub ecx, esi                      ; Subtract 0x9000 (address of start) + 24 (size of header) from it, to get the size.
    mov eax, 0xFFFFFFFF               ; Put the seed in EAX.
    
    call CRC32

    not eax                           ; Inverse the bits to get the CRC value.
    cmp eax, [esi - 4]                ; Compare the has with the hash stored in the file.
        
    jne .Error2                       ; Not equal? ERROR: Abort boot.

.ZeroBSS:
    mov esi, 0x9000 
    
    movzx edi, word [esi + 16]        ; Move the start of BSS section into EDI.
    movzx ecx, word [esi + 20]
    
    sub ecx, edi                      ; Calculate the length, and store it in ECX.
    shr ecx, 2                        ; Shift ecx right by 2, effectively dividing by 4.
    
    xor eax, eax                      ; Zero out EAX, since we want to clear the region.
    rep stosd                         ; Clear out the BSS section.
 
.JmpToBIOS:
    xor dx, dx
    mov ss, dx

    mov eax, OpenFile
    mov ebx, ReadFile
    mov ecx, CloseFile
    mov edx, BD_PXE
       
    mov esp, 0x7C00
    xor ebp, ebp
    jmp [0x9004]

.Error:
    xor ax, ax
    mov es, ax
    mov ds, ax

    mov si, PXEAPIError               ; Accessing the API error occured.
    jmp AbortBoot                     ; Abort boot now!

.Error2:
    mov si, BIOSError                 ; Unable to open/find the common BIOS file.
    jmp AbortBoot
