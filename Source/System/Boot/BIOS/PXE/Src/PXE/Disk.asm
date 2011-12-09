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

PXENV_TFTP_GET_FSIZE:
    .Status       dw 0
    .SIP          dd 0
    .GIP          dd 0
    .Filename     times 128 db 0
    .Filesize     dd 0

; Save PacketSize here, for future reference by code.
PacketSize:       dw 0
IsOpen:           db 0

Files:
    .BIOS         dw BIOSStr          ; Define the BIOS String in the table.
    .DBAL         dw DBALStr          ; And the DBAL string in the table.
    .Background   dw BackgroundStr    ; And the Background string in the table.

BIOSStr db "BIOS", 0
DBALStr db "DBAL", 0
BackgroundStr db "Background.sif", 0

SECTION .text

; Opens a file to be read from.
; @al             Contains the code number of the file to open.
;                 0 -> Common BIOS File.
;                 1 -> DBAL.
;                 2 -> Background image.
;     @rc 
;                 Returns with carry set if ANY error occured (technically, no error should be happening, but still).
;                 @ecx    The size of the file you want to open.
OpenFile:
    pushad

    ; Save AX - containing the file code - we will be using it later.
    push ax
    mov di, PXENV_TFTP_OPEN 
    mov ecx, 71                       ; Store 71-words.
    xor eax, eax                      ; We need to zero out the structure.

    rep stosw                         ; Clear out the entire structure.

    mov di, PXENV_TFTP_GET_FSIZE
    mov ecx, 71                       ; Store 71-words.
    xor eax, eax                      ; We need to zero out the structure.

    rep stosw                         ; Clear out the entire structure.

    cmp byte [IsOpen], 1              ; Check whether any file has already been opened or not.
    je .Error                         ; If yes, abort boot.

    mov byte [IsOpen], 1
 
    ; Put some default values in there.
    mov word [PXENV_TFTP_OPEN.Port], UDP_PORT
    mov word [PXENV_TFTP_OPEN.PacketSize], PACKET_SIZE
    mov eax, [SIP]
    mov [PXENV_TFTP_OPEN.SIP], eax 
    mov [PXENV_TFTP_GET_FSIZE.SIP], eax

    mov eax, [GIP]
    mov [PXENV_TFTP_OPEN.GIP], eax
    mov [PXENV_TFTP_GET_FSIZE.GIP], eax

    ; Store the "filename" at PXENV_TFTP_OPEN.Filename
    mov ecx, 32                       ; We need to store 32 dwords, or 128 bytes.

    pop ax                            ; And get back EAX.
    movzx ebx, al
    add ebx, ebx                      ; Double 'al' to get words.
    add bx, Files                     ; Add address to index.

    mov si, [bx]                      ; And then get the address into SI.

    mov di, PXENV_TFTP_OPEN.Filename
    rep movsd

    mov si, [bx]
    mov ecx, 32
    mov di, PXENV_TFTP_GET_FSIZE.Filename
    
    rep movsd                         ; And then get the filename into both the get file size, and open file structures.

    mov di, PXENV_TFTP_GET_FSIZE
    mov bx, TFTP_GET_FSIZE

    ; With error checking.
    call UsePXEAPI
    mov ax, [PXENV_TFTP_GET_FSIZE]
    test ax, ax
    jnz .Error

    ; Store the address of the input buffer, and the opcode at BX.
    mov di, PXENV_TFTP_OPEN
    mov bx, TFTP_OPEN
    call UsePXEAPI

    or ax, [PXENV_TFTP_OPEN]
    test ax, ax
    jnz .Error                        ; Test if any error occured. If it did, return with error.
    
    ; Store PacketSize for future reference.
    mov ax, [PXENV_TFTP_OPEN.PacketSize]
    mov [PacketSize], ax

.Return:        
    popad
    mov ecx, [PXENV_TFTP_GET_FSIZE.Filesize]
    ret

.Error:
    mov byte [IsOpen], 0
    stc
    popad
    ret

; Closes the previously opened file.
CloseFile:
    pushad

    cmp byte [IsOpen], 0
    je .Error

    mov byte [IsOpen], 0

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
    mov si, PXEAPIError
    jmp AbortBoot


; Reads the required bytes of the file currently opened.
; @edi            The destination address of where to read the file to.
; @ecx            The number of bytes to read.
;     @rc
;                 Aborts boot if any error occured (during read, that is).
ReadFile:
    pushad
 
.ReadFile: 
    mov edx, edi
    and edx, 0xFFFF0000
    shr edx, 4
    ; Get the segment in EDX.

    mov [PXENV_TFTP_READ.BufferOff], di
    mov [PXENV_TFTP_READ.BufferSeg], dx
    xor edx, edx

    mov word [PXENV_TFTP_READ.Status], 0
    mov word [PXENV_TFTP_READ.BufferSize], 0

    push edi                          ; Save DI for the moment (destination buffer address).

    mov di, PXENV_TFTP_READ
    mov bx, TFTP_READ
    call UsePXEAPI                    ; Use the API to read.

    pop edi                           ; And restore DI back again.
    
    cmp word [PXENV_TFTP_READ], 0x3B  ; If status is 0x3B - then FILE_NOT_FOUND error, which implies reading AFTER EOF.
    jne .Cont

    xor ax, ax
    mov word [PXENV_TFTP_READ], 0     ; If EOF, then no error code should be present, and bytes read should be 0.
    mov word [PXENV_TFTP_READ.BufferSize], 0  

.Cont:
    or ax, [PXENV_TFTP_READ] 
    test ax, ax
    jnz .Error                        ; If any error occured, abort boot.
    
    movzx edx, word [PXENV_TFTP_READ.BufferSize]
    test dx, dx                       ; If size read is zero, then EOF reached.
    jz .Return

    add edi, edx
    
    cmp ecx, edx
    jb .Return

    sub ecx, edx

    test ecx, ecx
    jnz .ReadFile

.Return:
    popad
    ret    

.Error:
    mov si, PXEAPIError
    jmp AbortBoot
