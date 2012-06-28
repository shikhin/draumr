 ; Contains functions related to the ISO9660 filesystem.
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

; Some error strings.
FilesNotFoundMsg:
    db "Required files (DBAL, BIOS, KL, Kernel, Modules) not present on disk.", EL, 0

; Save all LBA/sizes here.
Root:
    .LBA          dd 0                ; The LBA of the root directory.
    .Size         dd 0                ; The size of the root directory in bytes.

Boot:
    .LBA          dd 0                ; The LBA of the boot directory.
    .Size         dd 0                ; The size of the boot directory in bytes.

BIOS:
    .LBA          dd 0                ; The LBA of the BIOS file.
    .Size         dd 0                ; The Size of the BIOS file in bytes.

DBAL:
    .LBA          dd 0                ; The LBA of the DBAL file.
    .Size         dd 0                ; The Size of the DBAL file in bytes.

Background:
    .LBA          dd 0                ; The LBA of the Background file.
    .Size         dd 0                ; The size of the Background file in bytes.

KL:
    .LBA          dd 0                ; The LBA of the KL file.
    .Size         dd 0                ; The Size of the KL file in bytes.

Kernelx86:
    .LBA		  dd 0				  ; The LBA of the x86 kernel.
    .Size		  dd 0                ; The Size of the x86 kernel in bytes.

KernelAMD64:
    .LBA		  dd 0                ; The LBA of the AMD64 kernel.
    .Size		  dd 0                ; The Size of the AMD64 kernel in bytes.

PMMx86:
    .LBA      dd 0                    ; The size and the LBA of the PMM x86 module is unknown.
    .Size     dd 0

PMMx86PAE:
    .LBA      dd 0                    ; The size and the LBA of the PMM x86 PAE module is unknown.
    .Size     dd 0

PMMAMD64:
    .LBA      dd 0                    ; The size and the LBA of the PMM AMD64 module is unknown.
    .Size     dd 0

VMMx86:
    .LBA      dd 0                    ; The size and the LBA of the VMM x86 module is unknown.
    .Size     dd 0

VMMx86PAE:
    .LBA      dd 0                    ; The size and the LBA of the VMM x86 PAE module is unknown.
    .Size     dd 0

VMMAMD64:
    .LBA      dd 0                    ; The size and the LBA of the VMM AMD64 module is unknown.
    .Size     dd 0


FILE:
    .Code   db -1                     ; The "code" of the file opened. If -1, no file opened.
    .LBA    dd 0                      ; The LBA of the sector we are going to "read next".
    .Size   dd 0                      ; The size of the file left to read (as reported by the file system).
    .Extra  dd 0                      ; The number of "extra" bytes read in the last "transaction".
                                      ; And I'll just explain it over here. In cases of BIOS and DBAL file,
                                      ; we need to read exact on spot. Thus, if we read anything extra in 
                                      ; the last transaction, we carry that much over.
    
SECTION .text

 ; Checks the record in the boot directory for any matching files.
 ;     DI  -> the address of the record.
 ;
 ; Returns:
 ;     EBP -> decremented by one if file found.
CheckRecordBootDir:
    push esi
    push eax

    movzx eax, byte [di + 32]

    cmp eax, 5             ; Identifier size: 5.
    je .1

    cmp eax, 7             ; Identifier size: 7.
    je .2

    cmp eax, 8             ; Identifier size: 8.
    je .3

    cmp eax, 9             ; Identifier size: 9.
    je .4

    cmp eax, 10            ; Identifier size: 10.
    je .5

    cmp eax, 11            ; Identifier size: 11.
    je .6

    cmp eax, 14            ; Identifier size: 14.
    je .7

    ; None matches.
    jmp .Return

; Identifier size 5.
.1:
.1KL:
    ; Compare for KL.
    cmp word  [di + 33], "KL"
    jne .Return

    mov esi, KL
    jmp .FileFound

; Identifier size 7.
.2:
.2BIOS:
    ; Check if it's the common BIOS boot file.
    cmp dword [di + 33], "BIOS"
    jne .2DBAL

    ; If it is, load esi with BIOS and go to file found.
    mov esi, BIOS
    jmp .FileFound

.2DBAL:
    ; Check if it's the DBAL.
    cmp dword [di + 33], "DBAL"
    jne .Return

    ; If it is, go to file found.
    mov esi, DBAL
    jmp .FileFound

; Identifier size 8.
.3:
.3KEx86:
    ; Check if it's the kernel x86 file.
    cmp dword [di + 33], "KEX8"
    jne .Return

    cmp byte  [di + 37], '6'
    jne .Return

    ; If it is, go to file found.
    mov esi, Kernelx86
    jmp .FileFound

; Identifier size 9
.4:
.4PMMx86:
    ; Check if it's the PMM x86 module.
    cmp dword [di + 33], "PMMX"
    jne .4PMMPAE

    cmp word  [di + 37], "86"
    jne .Return

    ; If it is, go to file found.
    mov esi, PMMx86
    jmp .FileFound

.4PMMPAE:
    ; Check if it's the PMM PAE module.
    cmp dword [di + 33], "PMMP"
    jne .4VMMx86

    cmp word  [di + 37], "AE"
    jne .Return

    ; If it is, go to file found.
    mov esi, PMMx86PAE
    jmp .FileFound

.4VMMx86:
    ; Check if it's the VMM x86 module.
    cmp dword [di + 33], "VMMX"
    jne .4VMMPAE

    cmp word  [di + 37], "86"
    jne .Return

    ; If it is, go to file found.
    mov esi, VMMx86
    jmp .FileFound

.4VMMPAE:
    ; Check if it's the VMM PAE module.
    cmp dword [di + 33], "VMMP"
    jne .Return

    cmp word  [di + 37], "AE"
    jne .Return

    ; If it is, go to file found.
    mov esi, VMMx86PAE
    jmp .FileFound

; Identifier size 10.
.5:
.5KEAMD64:
    ; Check if it's the Kernel AMD64.
    cmp dword [di + 33], "KEAM"
    jne .Return

    cmp word  [di + 37], "D6"
    jne .Return

    cmp byte  [di + 39], '4'
    jne .Return

    ; If it is, go to file found.
    mov esi, KernelAMD64
    jmp .FileFound

; Identifier size 11.
.6:
.6PMMAMD64:
    ; Check if it's the PMM AMD64 module.
    cmp dword [di + 33], "PMMA"
    jne .6VMMAMD64

    cmp dword [di + 37], "MD64"
    jne .Return

    ; If it is, load esi with PMM AMD64 and go to file found.
    mov esi, PMMAMD64
    jmp .FileFound

.6VMMAMD64:
    ; Check if it's the VMM AMD64 module.
    cmp dword [di + 33], "VMMA"
    jne .Return

    cmp dword [di + 37], "MD64"
    jne .Return

    ; If it is, go to file found.
    mov esi, VMMAMD64
    jmp .FileFound

.7:
.7Background:
    ; Check if it's the background sif.
    cmp dword [di + 33], "BACK"
    jne .Return

    cmp dword [di + 37], "GROU"
    jne .Return

    cmp dword [di + 41], ".SIF"
    jne .Return

    ; If it is, go to file found.
    mov esi, Background
    jmp .FileFound

; If the file is found.
.FileFound:
    ; Store the LBA.
    mov eax, [di + 2]
    mov [esi], eax

    ; Store the size.
    mov eax, [di + 10]
    mov [esi + 4], eax

    ; Decrement EBP.
    dec ebp

; Return.
.Return:
    ; Pop all the registers.
    pop eax
    pop esi

    ret

 ; Is responsible for finding boot files.
 ;
 ; Returns: 
 ;     Boot -> aborted if ANY error occurs.
BootFilesFind:
    pushad    

    mov eax, [0x9000 + 156 + 10]      ; Get the size of the PVD root directory into EAX.
    mov ebx, [0x9000 + 156 + 2]       ; Get the LBA of the PVD root directory into EBX.

    ; Save the values.
    mov [Root.LBA], ebx
    mov [Root.Size], eax

    mov ecx, 1                        ; Only load 1 sector at a time.

.LoadSectorRD:
    mov edi, 0x9000 | 0x80000000      ; Enable advanced error checking.
    call DiskReadSector

.CheckRecordRD:
    cmp byte [di], 0                  ; If zero, we have finished this sector. Move on to next sector.
    je .NextSectorRD

    cmp byte [di + 32], 4             ; If size of directory identifier isn't 0, next record.
    jne .NextRecordRD

    cmp dword [di + 33], "BOOT"       ; If directory identifier doesn't match, next record.
    je .FoundBoot

.NextRecordRD:
    movzx edx, byte [di]              ; Save the size of the directory record into EDX.
    add di, dx                        ; Move to the next directory record.
    
    cmp di, 0x9800                    ; If we aren't below than 0x9000 + 2048, then we need to load the next sector.
    jb .CheckRecordRD 

.NextSectorRD:
    inc ebx                           ; Increase the LBA.
    
    sub eax, 0x800                    ; Decrease number of bytes left.
    jnz .LoadSectorRD                 ; If EAX isn't zero, load next sector and continue.

.FoundBoot:
    mov eax, [di + 10]
    mov ebx, [di + 2]

    ; Save some values we probably'd need later on.
    mov [Boot.LBA], ebx
    mov [Boot.Size], eax
    
    mov ebp, 12                       ; Number of files to load.

.LoadSectorBD:
    mov edi, 0x9000 | 0x80000000      ; Enable advanced error checking.
    call DiskReadSector

.CheckRecordBD:
    cmp byte [di], 0                  ; If zero, we have finished this sector. Move on to next sector.
    je .NextSectorBD

    ; Check the record in the boot dirctory.
    call CheckRecordBootDir

.CheckForAllDone:
    test ebp, ebp
    ; If found all files, return.
    jz .Return

.NextRecordBD:
    movzx edx, byte [di]              ; Save the size of the directory record into EDX.
    add di, dx                        ; Move to the next directory record.
    
    cmp di, 0x9800                    ; If we aren't below than 0x9000 + 2048, then we need to load the next sector.
    jb .CheckRecordBD 

.NextSectorBD:
    inc ebx                           ; Increase the LBA.
    
    sub eax, 0x800                    ; Decrease number of bytes left.
    jnz .LoadSectorBD                 ; If EAX isn't zero, load next sector and continue.

    jmp .NotFound                     ; If we reached here, we haven't found all the files. Abort.
  
; Not found - abort boot.
.NotFound:
    ; If the only thing we haven't found yet is the background image, then, can continue.
    cmp ebp, 1
    jg .Abort

    ; So 1 file hasn't been found. Is it the image?
    cmp dword [Background.LBA], 0
    je .Return                        ; Yes, return.

.Abort:
    ; Else, abort.
    mov si, FilesNotFoundMsg
    jmp AbortBoot

.Return:
    popad
    ret

 ; Opens a file to be read from.
 ;     AL    -> contains the code number of the file to open.
 ;      0    -> Common BIOS File.
 ;      1    -> DBAL.
 ;      2    -> Background Image.
 ;      3    -> KL.
 ;      4    -> Kernel x86.
 ;      5    -> Kernel AMD64.
 ;      6    -> PMM x86.
 ;      7    -> PMM x86 PAE.
 ;      8    -> PMM AMD64.
 ;      9    -> VMM x86.
 ;      10   -> VMM x86 PAE.
 ;      11   -> VMM AMD64.
 ;
 ; Returns: 
 ;     Carry -> set if any error occured.
 ;     ECX   -> the size of the file you want to FILE.
FileOpen:
    push eax
    push esi

    ; If file code isn't -1, then return with error.
    cmp byte [FILE.Code], -1
    jne .Error

    ; If it is above the maximum file code (11), go to error.
    cmp al, 11
    ja .Error

    ; Store the file code.
    mov [FILE.Code], al

    ; Get the file code in ESI, and multiply by 3.
    movzx esi, al
    shl esi, 3

    ; Add address of BIOS file descriptor.
    add esi, BIOS

    ; Store the LBA and Size in the open file descriptor.
    mov eax, [esi]
    mov [FILE.LBA], eax

    mov eax, [esi + 4]
    mov [FILE.Size], eax

    cmp byte [FILE.Code], 2
    jne .Return

    mov eax, [Background.LBA]
    test eax, eax
    jz .Error

.Return:
    ; Restore registers.
    pop esi
    pop eax
    clc

    mov ecx, [FILE.Size] 
    ret

.Error:
    ; Set the file code to -1, and the carry flag.
    mov byte  [FILE.Code], -1
    mov dword [FILE.Extra], 0

    stc

    pop esi
    pop eax
    ret

 ; Reads the 'next LBA' of the file currently opened.
 ;     EDI  -> the destination address of where to read the file to.
 ;     ECX  -> the number of bytes to read.
 ;
 ; Returns:
 ;     Boot -> aborted if any error occured.
FileRead:
    pushad

    mov edx, ecx                      ; Get the original number of bytes in EDX.
    add ecx, 0x7FF
    and ecx, ~0x7FF                   ; Get it to the highest rounded 0x800 byte thingy.

    mov eax, ecx                      ; Get the new number of bytes in EAX.
    sub eax, edx                      ; And now, get the extra in EAX.
    push eax                          ; Push it.

    ; Get the LBA in EBX.
    mov ebx, [FILE.LBA]

    cmp ecx, [FILE.Size]              ; If size we want to read <= size we can read continue;
    jbe .Cont
  
    mov ecx, [FILE.Size]              ; Else, we read only [FILE.Size] bytes.
    
    ; If which is zero, we return.
    test ecx, ecx
    jz .Return

.Cont:
    sub [FILE.Size], ecx              ; Subtract bytes read from bytes we can read.

.Read:
    ; Add the extra bytes.
    add edi, [FILE.Extra]
    
    add ecx, 0x7FF
    shr ecx, 11                       ; And the number of sectors to read in ECX.

    mov edx, ecx                      ; Keep that for internal count.

; Here we have the number of sectors to read in ECX, the LBA in EAX and the destination buffer in EDI. Let's shoot!
.Loop:
    call DiskReadSectorM              ; Do the CALL!
    
    add ebx, ecx                      ; Advance the LBA by read sectors count.
   
    sub edx, ecx                      ; EDX more sectors left to do.
    jz .Return                        ; Read all sectors, return.
  
    ; Now need to advance EDI.
    mov ebp, ecx
    shl ebp, 12
    add edi, ebp
    
    mov ecx, edx                      ; If not, read EDX (sectors left to do) sectors next time.
    jmp .Loop

.Return:
    mov [FILE.LBA], ebx
    
    pop eax
    mov [FILE.Extra], eax
    
    popad
    ret

 ; Closes the file currently opened.
FileClose:
    mov byte [FILE.Code], -1
    mov dword [FILE.Extra], 0
    mov dword [FILE.Size], 0
    mov dword [FILE.LBA], 0
    ret
