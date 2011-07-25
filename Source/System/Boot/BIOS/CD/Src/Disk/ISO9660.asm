; Contains functions related to the ISO9660 filesystem.
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


; Finds the Primary Volume Descriptor, puts its LBA in PVDLBA and loads it at 0x9000.
;     @rc
;                 Aborts boot if any error occurs.
FindPVD:
    pushad
    xor si, si                        ; If any error occurs, basic boot.
    mov ecx, 1                        ; Only read one sector (2KiB).
    mov ebx, 0x0F                     ; We should start from the 16th Sector.
    mov edi, 0x9000 | 0x80000000      ; Read the sector at 0x9000 WITH advanced disk checking.

.LoopThroughSectors:
    inc ebx
    call ReadFromDisk                 ; Read the disk.
    jc AbortBoot                      ; The read from the disk failed for some reason.

    cmp byte [di], 0x1                ; If the Type field contains 0x1 we just stumbled upon the Primary Volume Descriptor.
    jne .Next 

    cmp dword [di + 1], 'CD00'        ; Check if valid Volume Descriptor.
    jne .Next                         ; No, move on to next.

    cmp byte [di + 4], '1'            ; Check if valid Volume Descriptor.
    jne .Next                         ; No, move on to next. 

.Next:
    cmp byte [di], 255                ; This is the Volume Descriptor Set Terminator. No more Volume Descriptors.
    je AbortBoot                      ; The PVD isn't present! :-(

    jmp .LoopThroughSectors

.Found:
    popad
    mov [BootInfo.PVD], ebx
    ret


; Some error strings.
FilesNotFound     db "ERROR: Important boot files are not present.", nl, 0

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

; Is responsible for finding boot files.
;     @rc
;                 Aborts boot if ANY error occurs.
FindBootFiles:
    pushad    

    mov eax, [0x9000 + 156 + 10]      ; Get the size of the PVD root directory into EAX.
    mov ebx, [0x9000 + 156 + 2]       ; Get the LBA of the PVD root directory into EBX.

    ; Save the values.
    mov [Root.LBA], ebx
    mov [Root.Size], eax

    mov ecx, 1                        ; Only load 1 sector at a time.

.LoadSectorRD:
    mov edi, 0x9000 | 0x80000000      ; Enable advanced error checking.
    call ReadFromDisk

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
    test eax, eax
    jnz .LoadSectorRD                 ; If EAX isn't zero, load next sector and continue.

.FoundBoot:
    mov eax, [di + 10]
    mov ebx, [di + 2]

    ; Save some values we probably'd need later on.
    mov [Boot.LBA], ebx
    mov [Boot.Size], eax
    
    mov edx, "BIOS"
    mov ebp, 1                        ; Number of files to load.

.LoadSectorBD:
    mov edi, 0x9000 | 0x80000000      ; Enable advanced error checking.
    call ReadFromDisk

.CheckRecordBD:
    cmp byte [di], 0                  ; If zero, we have finished this sector. Move on to next sector.
    je .NextSectorBD

    cmp byte [di + 32], 7             ; If size of directory identifier isn't 0, next record.
    jne .NextRecordBD

    cmp dword [di + 33], "BIOS"       ; If directory identifier doesn't match, next record.
    je .FoundBIOS

.NextRecordBD:
    movzx edx, byte [di]              ; Save the size of the directory record into EDX.
    add di, dx                        ; Move to the next directory record.
    
    cmp di, 0x9800                    ; If we aren't below than 0x9000 + 2048, then we need to load the next sector.
    jb .CheckRecordBD 

.NextSectorBD:
    inc ebx                           ; Increase the LBA.
    sub eax, 0x800                    ; Decrease number of bytes left.
    test eax, eax
    jnz .LoadSectorBD                 ; If EAX isn't zero, load next sector and continue.

.FoundBIOS:
    push eax
    push ebx
 
    mov eax, [di + 10]
    mov ebx, [di + 2]

    mov [BIOS.LBA], ebx
    mov [BIOS.Size], eax

    pop ebx
    pop eax

    dec ebp

.MoveOn:
    test ebp, ebp
    jnz .NextRecordBD

    jmp .Return
  
; Not found - abort boot.
.NotFound:
    mov si, FilesNotFound
    mov ax, 0
    call AbortBoot

.Return:
    popad
    ret
