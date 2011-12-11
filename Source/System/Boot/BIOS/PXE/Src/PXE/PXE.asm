; Contains functions for initializing usage of the PXE API.
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

NoPXE       db    "ERROR: Not booted from PXE, or invalid PXE.", 0
PXEAPIError db    "ERROR: Error occured while trying to access PXE API.", 0 
SIP         dd 0
GIP         dd 0

ALIGN 4
PXEAPI:
    .Offset    dw 0                   ; Offset of PXE API.
    .Segment   dw 0                   ; Segment of PXE API.

%define GET_CACHED_INFO 0x71
%define TFTP_OPEN       0x20
%define TFTP_CLOSE      0x21
%define TFTP_READ       0x22
%define TFTP_GET_FSIZE  0x25

SECTION .text

; Calls the PXE API, and abstracts things such that it works on both old and new APIs.
; @ds:di          The address of the input buffer.
; @bx             The opcode.
; NOTE: The above registers are what the legacy API also used, so it should cause no problem with it.
UsePXEAPI:
    ; And we push it over here, so that we have no problems with the new API.
    push ds
    push di
    push bx
    
    call far [PXEAPI]
    
    add sp, 6                         ; Clean up the stack.
    ret

; Initializes PXE (and checks whether the structures are correct or not).
; @es:bx          Points to the PXENV+ structure.
; @ss:sp + 4      Points to the !PXE structure (if valid).
InitPXE:
    pushad
   
    ; If the signature isn't equal, abort boot.
    cmp dword [es:bx], "PXEN" 
    jne .NoPXE

    cmp word [es:bx + 4], "V+"
    jne .NoPXE

    movzx ecx, byte [es:bx + 8]
    xor eax, eax
    push bx                           ; Save BX by pushing it on the stack.

; Now ECX contains the number of bytes on which to do the checksum, and EAX would contain the result.
.Checksum:
    add al, byte [es:bx]              ; Add the byte at es:bx to result.
    inc bx                            ; Point to next byte.
   
    dec cx                            ; Decrement number of bytes left.
    jnz .Checksum                     ; Done all bytes - return!

    test al, al
    jnz .NoPXE                        ; If result was not zero - no PXE!

    pop bx                            ; Get BX back - by poping it from the stack.

    cmp word [es:bx + 6], 0x0201      ; Compare the version.
    jg .PXE                           ; If greater; use !PXE, else be content with PXENV+.

.PXENV:
    mov ecx, [es:bx + 0x0A]

    push ds
    xor ax, ax
    mov ds, ax
    mov [ds:PXEAPI], ecx              ; Save the API address for future use.
    pop ds

    jmp .Return

.PXE:
    ; Save ES and BX (in case we need to switch back to PXENV)
    push es
    push bx
 
    ; Reload ES:BX to point to !PXE structure.
    mov ax, ss
    mov es, ax
    mov bx, sp
    add bx, 4

    cmp dword [es:bx], "!PXE"         ; Check the signature.
    jne .ErrorPXE

    movzx ecx, byte [es:bx + 4]       ; Get the length of the structure.
    xor eax, eax
    push bx                           ; Save BX by pushing it on the stack.

; Now ECX contains the number of bytes on which to do the checksum, and EAX would contain the result.
.Checksum2:
    add al, byte [es:bx]              ; Add the byte at es:bx to result.
    inc bx                            ; Point to next byte.
    
    dec cx                            ; Decrement number of bytes left.
    jnz .Checksum2                    ; Done all bytes - return!

    test al, al
    jnz .FailPXEChecksum              ; If result was not zero - no PXE!

    pop bx                            ; Get BX back - by poping it from the stack.

    mov ecx, [es:bx + 0x10]           ; Get the address of the API.
    push ds

    xor ax, ax
    mov ds, ax
    mov [ds:PXEAPI], ecx              ; Save the API for future use.

    pop ds
    ; Restore ES and BX.
    pop bx
    pop es

.Return:
    popad
    ret

.FailPXEChecksum:
    pop bx

.ErrorPXE:
    ; Restore ES and BX, and try PXENV+ structure.
    pop bx
    pop es
    jmp .PXENV

.NoPXE:
    ; Mov the address of the string into SI, and zero out DS and ES, before aborting boot.
    xor ax, ax
    mov ds, ax                        ; Clear out ES and DS.
    mov es, ax
 
    mov si, NoPXE
    jmp AbortBoot   
