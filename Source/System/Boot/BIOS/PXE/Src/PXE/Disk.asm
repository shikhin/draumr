; Contains functions for reading from Disk using the PXE API.
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

%define PACKET_SIZE 512               ; Keep the size to the minimum and most supported 512.
%define UDP_PORT    (69 << 8)         ; The port is 69, or 69 << 8 in big endian.

PXENV_TFTP_OPEN:
    .Status       dw 0
    .SIP          dd 0
    .GIP          dd 0
    .Filename     times 128 db 0
    .Port         dw 0
    .PacketSize   dw 0


PXENV_TFTP_READ:
    .Status       dw 0
    .PacketNumber dw 0
    .BufferSize   dw 0
    .BufferOff    dw 0
    .BufferSeg    dw 0

; Save PacketSize here, for future reference by code.
PacketSize:       dw 0

SECTION .text


; Opens a file from the TFTP server, so that it can be read from.
; @si             Should point to the asciiz filename.
; @ecx            Should contain the length of the filename (< 128).
OpenFile:
    pushad
    
    ; Zero out the PXENV_TFTP_OPEN structure.
    push ecx                          ; Save the size of the filename.
    mov di, PXENV_TFTP_OPEN 
    mov ecx, 71                       ; Store 71-words.
    xor eax, eax                      ; We need to zero out the structure.

    rep stosw                         ; Clear out the entire structure.

    pop ecx                           ; Restore the size of the filename.
 
    ; Put some default values in there.
    mov word [PXENV_TFTP_OPEN.Port], UDP_PORT
    mov word [PXENV_TFTP_OPEN.PacketSize], PACKET_SIZE
    mov eax, [SIP]
    mov [PXENV_TFTP_OPEN.SIP], eax 

    mov eax, [GIP]
    mov [PXENV_TFTP_OPEN.GIP], eax

    ; Store the "filename" at PXENV_TFTP_OPEN.Filename
    mov di, PXENV_TFTP_OPEN.Filename
    rep movsb

    ; Store the address of the input buffer, and the opcode at BX.
    mov di, PXENV_TFTP_OPEN
    mov bx, TFTP_OPEN
    call UsePXEAPI

    or ax, [PXENV_TFTP_OPEN]
    test ax, ax
    jnz .Error                        ; Test if any error occured. If it did, abort boot!

    ; Store PacketSize for future reference.
    mov eax, [PXENV_TFTP_OPEN.PacketSize]
    mov [PacketSize], eax

.Return:
    popad
    ret

.Error:
    xor ax, ax
    mov si, PXEAPIError
    call AbortBoot

; Closes the previously opened file - if no file was previous opened, expect errors.
CloseFile:
    pushad

    mov di, PXENV_TFTP_OPEN
    mov bx, TFTP_CLOSE
    call UsePXEAPI

    or ax, [PXENV_TFTP_OPEN]
    test ax, ax
    jnz .Error                        ; Test if any error occured. 

.Return:
    popad
    ret

.Error:
    xor ax, ax
    mov si, PXEAPIError
    call AbortBoot


; Read a file completely off to some buffer.
; @di             Should point to the output buffer (where to read the file to).
; @ecx            Should contain the length of "filename".
; @si             Should point to "filename".
ReadFile:
    pushad

    ; Open the file.
    call OpenFile

.ReadFile: 
    mov [PXENV_TFTP_READ.BufferOff], di
    mov word [PXENV_TFTP_READ.Status], 0
    mov word [PXENV_TFTP_READ.BufferSize], 0

    push di

    mov di, PXENV_TFTP_READ
    mov bx, TFTP_READ
    call UsePXEAPI

    pop di

    or ax, [PXENV_TFTP_READ]
    test ax, ax
    jnz .Error

    add di, [PacketSize]
    mov cx, [PXENV_TFTP_READ.BufferSize]
    cmp cx, [PacketSize]
    je .ReadFile

    ; Close the file.
    call CloseFile
    
.Return:
    popad
    ret    

.Error:
    xor ax, ax
    mov si, PXEAPIError
    call AbortBoot
