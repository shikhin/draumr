 ; Contains functions for reading from Disk using the PXE API.
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

; Code of the file currently opened.
FILE:
    .Code:             db -1
    .Size:             dd 0
    
Files:
    .BIOS         dw BIOSStr          ; Define the BIOS String in the table.
    .DBAL         dw DBALStr          ; And the DBAL string in the table.
    .Background   dw BackgroundStr    ; And the Background string in the table.

ALIGN 4
; Reserve some space to read *one* packet (512) to find out the size of the file.
FirstPacket:
    times PACKET_SIZE db 0
    
; A flag to signify first packet - 1 if yes, 0 if not.
FirstPacketFlag: db 0
    
BIOSStr db "BIOS", 0
DBALStr db "DBAL", 0
BackgroundStr db "Background.sif", 0

SECTION .text

 ; Opens a file to be read from.
 ;     AL     -> contains the code number of the file to open.
 ;      0     -> Common BIOS File.
 ;      1     -> DBAL.
 ;      2     -> Background image.
 ;
 ; Returns: 
 ;      Carry -> set if ANY error occured (technically, no error should be happening, but still).
 ;      ECX   -> the size of the file you want to open.
FileOpen:
    pushad

    cmp byte [FILE.Code], -1               ; Check whether any file has already been opened or not.
    jne .Error                         ; If yes, abort boot.

    mov byte [FILE.Code], al

    ; Save AX - containing the file code - we will be using it later.
    push ax
    mov di, PXENV_TFTP_OPEN 
    mov ecx, 71                       ; Store 71-words.
    xor eax, eax                      ; We need to zero out the structure.

    rep stosw                         ; Clear out the entire structure.
     
    ; Put some default values in there.
    mov word [PXENV_TFTP_OPEN.Port], UDP_PORT
    mov word [PXENV_TFTP_OPEN.PacketSize], PACKET_SIZE
    mov eax, [SIP]
    mov [PXENV_TFTP_OPEN.SIP], eax 
    
    mov eax, [GIP]
    mov [PXENV_TFTP_OPEN.GIP], eax
    
    ; Store the "filename" at PXENV_TFTP_OPEN.Filename
    mov ecx, 32                       ; We need to store 32 dwords, or 128 bytes.

    pop ax                            ; And get back EAX.
    movzx ebx, al
    add ebx, ebx                      ; Double 'al' to get words.
    add bx, Files                     ; Add address to index.

    mov si, [bx]                      ; And then get the address into SI.
    
    mov di, PXENV_TFTP_OPEN.Filename
    rep movsd
    
    ; Store the address of the input buffer, and the opcode at BX.
    mov di, PXENV_TFTP_OPEN
    mov bx, TFTP_OPEN
    call PXEAPICall

    or ax, [PXENV_TFTP_OPEN]
    jnz .Error                        ; Test if any error occured. If it did, return with error.
    
    mov word [PXENV_TFTP_READ.Status], 0
    mov word [PXENV_TFTP_READ.PacketNumber], 0
    mov word [PXENV_TFTP_READ.BufferSize], 0
    mov word [PXENV_TFTP_READ.BufferSeg], 0
    
    ; Read the first packet.
    mov word [PXENV_TFTP_READ.BufferOff], FirstPacket

    mov di, PXENV_TFTP_READ
    mov bx, TFTP_READ
    call PXEAPICall                    ; Use the API to read.

    ; If the call failed, then simply close the file and return.
    or ax, [PXENV_TFTP_READ]
    jz .Cont
    
    ; Close the file, and then, return with carry set.
    call FileClose
    jmp .Error
    
.Cont:    
    mov byte [FirstPacketFlag], 1
    call GetFileSize
    mov [FILE.Size], ecx
    
.Return:        
    popad
    
    mov ecx, [FILE.Size]
    ret

.Error:
    mov byte [FILE.Code], -1
    stc
    
    popad
    ret

 ; Gets the file size of the file whose first packet is in FirstPacket.
 ;
 ; Returns:
 ;     ECX -> the size of the file, 0 if undetectable file format.
GetFileSize:
    ; Clear out ecx, so we return with "undetectable" if can't detect.
    xor ecx, ecx
    
    cmp dword [FirstPacket], "BIOS"
    je .BootFiles

    cmp dword [FirstPacket], "DBAL"
    je .BootFiles

    cmp word [FirstPacket], "SI"
    jne .Return

    cmp byte [FirstPacket + 2], "F"
    je .BackgroundImg

.Return:
    ; Return if not matched with anything.
    ret

; So it is one of the boot files - with the same format.
.BootFiles:
    ; Get end of file in @ecx, and then, subtract start of file.
    mov ecx, [FirstPacket + 12]
    sub ecx, [FirstPacket + 8]
    
    ret
   
; If matched with background image.
.BackgroundImg:
    ; Put the length into ecx.
    mov ecx, [FirstPacket + 3]
    ret

 ; Reads the required bytes of the file currently opened.
 ;     EDI   -> the destination address of where to read the file to.
 ;     ECX   -> the number of bytes to read.
 ;
 ; Returns:
 ;     Abort -> boot is aborted if any error occured (during read, that is).
FileRead:
    pushad

    ; If it isn't the first packet we are trying to read - just read the rest.
    cmp byte [FirstPacketFlag], 0
    je .FileRead
    
.FirstPacket:
    mov byte [FirstPacketFlag], 0
    
    ; Save edi and ecx.
    push edi
    push ecx
    push es
        
    mov edx, edi
    and edi, 0x000F
    shr edx, 4
    mov es, dx
           
    ; Copy the first packet out.
    mov ecx, PACKET_SIZE / 4
    mov esi, FirstPacket
    
    rep movsd
    
    ; Restore them.
    pop es
    pop ecx
    pop edi
 
    ; Clear out status, and set BufferSize to what we just copied.
    mov word [PXENV_TFTP_READ.Status], 0
    mov word [PXENV_TFTP_READ.BufferSize], PACKET_SIZE
    xor ax, ax
    
.Cont:
    or ax, [PXENV_TFTP_READ] 
    jnz .Error                        ; If any error occured, abort boot.
    
    movzx edx, word [PXENV_TFTP_READ.BufferSize]
    test dx, dx                       ; If size read is zero, then EOF reached.
    jz .Return

    add edi, edx

    cmp ecx, edx
    jbe .Return

    sub ecx, edx
    
.FileRead: 
    mov edx, edi
    mov ebx, edi
    and ebx, 0x000F
    shr edx, 4
    ; Get the segment in EDX, offset in EBX.

    mov [PXENV_TFTP_READ.BufferOff], bx
    mov [PXENV_TFTP_READ.BufferSeg], dx
    
    mov word [PXENV_TFTP_READ.Status], 0
    mov word [PXENV_TFTP_READ.BufferSize], 0

    push edi                          ; Save DI for the moment (destination buffer address).
    push ecx
        
    mov di, PXENV_TFTP_READ
    mov bx, TFTP_READ
    call PXEAPICall                    ; Use the API to read.

    pop ecx
    pop edi                           ; And restore DI back again.
    
    cmp word [PXENV_TFTP_READ], 0x3B  ; If status is 0x3B - then FILE_NOT_FOUND error, which implies reading AFTER EOF (or so found out with few machines).
    jne .Cont

    xor ax, ax
    mov word [PXENV_TFTP_READ], 0     ; If EOF, then no error code should be present, and bytes read should be 0.
    mov word [PXENV_TFTP_READ.BufferSize], 0  

    jmp .Cont

.Return:    
    popad
    ret    

.Error:
    mov si, PXEAPIError
    jmp AbortBoot

 ; Closes the previously opened file.
FileClose:
    pushad

    cmp byte [FILE.Code], -1
    je .Return

    mov byte [FILE.Code], -1

    mov di, PXENV_TFTP_OPEN
    mov bx, TFTP_CLOSE
    call PXEAPICall

.Return:
    popad
    ret