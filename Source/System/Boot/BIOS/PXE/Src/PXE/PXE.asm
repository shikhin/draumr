 ; Contains functions for initializing usage of the PXE API.
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

SECTION .data

ErrorPXENotPresentMsg:
    db "The Preboot eXecution Environment is not present or invalid.", EL, 0

ErrorPXEAPIMsg:
    db "Unable to access the PXE API.", EL, 0

ErrorPXECleanupMsg:
    db "Unable to cleanup PXE leftovers.", EL, 0

SIP         dd 0
GIP         dd 0

ALIGN 4
PXEAPI:
    .Offset    dw 0                   ; Offset of PXE API.
    .Segment   dw 0                   ; Segment of PXE API.

%define PXE_NEW_PRESENT (1 << 0)

PXEFlags       db 0                   ; Some flags.

PXENV_CLEANUP:
    .Status    dw 0                   ; The status.
    .Reserved  times 10 db 0 

%define UNDI_SHUTDOWN   0x05
%define UNLOAD_STACK    0x70
%define STOP_UNDI       0x15
%define UNDI_CLEANUP    0x02

%define GET_CACHED_INFO 0x71
%define TFTP_OPEN       0x20
%define TFTP_CLOSE      0x21
%define TFTP_READ       0x22
%define TFTP_GET_FSIZE  0x25

SECTION .text

 ; The PXE cleanup function, which aborts all PXE services.
PXECleanup:
    pushad

    ; Reset the network adapter and leave it in a safe state.
    mov bx, UNDI_SHUTDOWN
    mov di, PXENV_CLEANUP
    call PXEAPICall

    ; Abort boot if error.
    or ax, [PXENV_CLEANUP]
    jnz .Error

    ; Unload the stack.
    mov bx, UNLOAD_STACK
    mov di, PXENV_CLEANUP
    call PXEAPICall

    ; Abort boot if error.
    or ax, [PXENV_CLEANUP]
    jnz .Error

    cmp byte [PXEFlags], PXE_NEW_PRESENT
    jne .Cont

; PXE! -> use STOP_UNDI.
    mov bx, STOP_UNDI
    mov di, PXENV_CLEANUP
    call PXEAPICall

    ; Abort boot if error.
    or ax, [PXENV_CLEANUP]
    jnz .Error

    jmp .Return

; Use UNDI_CLEANUP instead.
.Cont:
    mov bx, UNDI_CLEANUP
    mov di, PXENV_CLEANUP
    call PXEAPICall

    ; Abort boot if error.
    or ax, [PXENV_CLEANUP]
    jnz .Error

.Return:
    popad
    ret

.Error:
    ; Set to mode 0x03, or 80*25 text mode.
    mov ax, 0x03
   
    ; SWITCH!
    int 0x10

    mov si, ErrorPXECleanupMsg
    jmp AbortBoot

 ; Calls the PXE API, and abstracts things such that it works on both old and new APIs.
 ;     DS:DI -> the address of the input buffer.
 ;     BX    -> the opcode.
; NOTE: The above registers are what the legacy API also used, so it should cause no problem with it.
PXEAPICall:
    ; And we push it over here, so that we have no problems with the new API.
    push ds
    push di
    push bx
    
    call far [PXEAPI]
    
    add sp, 6                         ; Clean up the stack.
    
    ret

 ; Initializes PXE (and checks whether the structures are correct or not).
 ;     ES:BX     -> points to the PXENV+ structure.
 ;     SS:SP + 4 -> points to the !PXE structure (if valid).
PXEInit:
    pushad
   
    ; If the signature isn't equal, abort boot.
    cmp dword [es:bx], "PXEN" 
    jne .ErrorPXENotPresent

    cmp word [es:bx + 4], "V+"
    jne .ErrorPXENotPresent

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
    jnz .ErrorPXENotPresent           ; If result was not zero - no PXE!

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

    ; So we're using the PXE! structure instead of PXENV+ structure.
    or byte [PXEFlags], PXE_NEW_PRESENT

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

.ErrorPXENotPresent:
    ; Mov the address of the string into SI, and zero out DS and ES, before aborting boot.
    xor ax, ax
    mov ds, ax                        ; Clear out ES and DS.
    mov es, ax
 
    mov si, ErrorPXENotPresentMsg
    jmp AbortBoot   