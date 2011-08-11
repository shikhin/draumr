; Contains functions to access the Disk.
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

; This section contains all the macros.
%define           nl 0x0A, 0x0D
DiskError db "ERROR: Unable to access the boot floppy drive.", nl, 0


SECTION .base


; Save the boot drive number here.
BootDrive         dd 0

Retry             db 0                ; Number of times to retry a particular function - safety purposes.

FindPVDMan        db 0                ; Set to 1 if need to find PVD by basic method.


; Gets the complete the boot file (us).
GetBootFile:
    pushad

.Return:    
    popad
    ret
 

; Read a sector from the disk to a buffer.
;     @edi        The buffer to where to read the disk sector to.

;     @ebx        The LBA address from where to read to.
;     @ecx        The number of sectors to read.
;     @rc
;                 Aborts boot if unsuccesful.
ReadFromDisk:
    mov byte [Retry], 3
    pushad

.Retry:


.Success:
    popad
    ret


SECTION .text


; Reads multiple sectors from disk.
; Expects same arguments as that for the above.
; M stands for Multiple (sectors).
ReadFromDiskM:
    pushad
   
    ; Save the count of sectors to read in EAX. 
    mov eax, ecx
    ; And read on sector at a time - reading more sectors cause a lot of problems usually.
    mov ecx, 1

.Single:
    call ReadFromDisk
    
    ; Decrease the count of sectors to read.
    dec eax
    ; If read all sectors, end.
    test eax, eax
    jz .Return
    
    inc ebx                           ; Increase the LBA.
    add di, 0x800                     ; And the destination buffer.
    jmp .Single 

.Return:
    popad
    ret


; Prints out error messages reported by the BIOS on disk operations.
; @ah             Is supposed to contain the error code.
;     @rc
;     @si         Contains the address of the string to print (when disk error occured).
;     @ax         The type of beep to produce.
GetErrorMsg:
    test ah, ah                       ; If AH is zero, print a generic error message.
    je .Generic

    xor si, si
    movzx ebx, ah
    xor ax, ax                        ; TODO: This should contain the code for disk error.

.Table1:
    cmp bx, 0x10
    jg .Table2                        ; If AH doesn't fit in table 1, try table 2.

    ; Find the string address in the Disk Error Table.
    add bx, bx                        ; Double AX, since each address is a word and not a byte.
    add bx, DiskErrTable1             ; Add the address of the DiskErrTable1 to it
    
    mov dx, [bx]                      ; Get the address of the string into DX.
 
    ; If DX is zero go to the generic case.
    test dx, dx
    je .Generic

    ; Print the error message.
    mov si, dx
    ret

.Table2:
    cmp bx, 0xB0
    jb .Special20                     ; If it is smaller than 0xB0, we can try the special cases.
    cmp bx, 0xB6
    jg .Generic                       ; Else, the generic ones.

    ; Find the string address in the Disk Error Table.
    sub bx, 0xB0                      ; Subtract 0xB0 to find the offset.
    add bx, bx                        ; Double AX, since each address is a word and not a byte.
    add bx, DiskErrTable2             ; Add the address of the DiskErrTable2 to it
  
    mov dx, [bx]
 
    ; If DX is zero go to the generic case.
    test dx, dx
    je .Generic

    ; Print the error message.
    mov si, dx
    ret

; NOTE: For all cases where I didn't put the error messages in a table
; so as to decrease the size of the table, I test for them in a Special
; Case clause.

; Special case for error code 0x20.
.Special20:
    cmp bx, 0x20
    jne .Special31

    mov si, DiskErrCmd20
    ret

; Special case for error code 0x31.
.Special31:
    cmp bx, 0x31
    jne .Special40

    mov si, DiskErrCmd31
    ret

; Special case for error code 0x40.
.Special40:
    cmp bx, 0x40
    jne .Special80

    mov si, DiskErrCmd40
    ret

; Special case for error code 0x80.
.Special80:
    cmp bx, 0x80
    jne .Generic

    mov si, DiskErrCmd80
    ret

.Generic:
    mov si, DiskErr0
    ret


