; Responsible for finding the ACPI, MPS and SMBIOS tables.
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

BITS 32

; Finds the ACPI, MPS and SMBIOS tables, and stores them in the BIT.
;     If unable to find any one of them, it lets the address remain to a default 0:0.
FindTables:
    pushad

    movzx esi, word [0x40E]
    shl esi, 4                        ; Usually, the word at 0x40E contains the address of EBDA shifted right by 4.
    
    test esi, esi                     ; If ESI contains 0, then assume that no EBDA is present, or something.
    jz .Cont
   
    ; Check the first kilobyte of the EBDA.
    mov ecx, 0x400
    
; We have the starting address in ESI, and the size to check in ECX.
.EBDA:
; Check if we got the RSDP.
.EBDARSDP:
    cmp [esi], dword "RSD "
    jne .EBDAMPS

    cmp [esi + 4], dword "PTR "
    jne .EBDAMPS

; Check the checksum - this should pass, else we don't consider it to be valid.
.ChecksumRSDP:
    push ecx

    ; The RSDP only needs the checksum of the first 20 bytes.
    mov ecx, 20
    call Checksum
   
    pop ecx
   
    ; A zero in AL means that we got clean.
    test al, al
    jnz .EBDAMPS

    mov [BIT.ACPI], esi
    jmp .EBDANext

.EBDAMPS:    
    cmp [esi], dword "_MP_"
    jne .EBDANext

; The checksum of the MPS tables.
.ChecksumMPS:
    push ecx                          ; Save ECX - since it's used in the Checksum process.

    ; The length is at offset 8 - in 16 byte units.
    movzx eax, byte [esi + 8]
    mov ecx, 0x10                     ; Multiple it by 0x10.
    mul ecx                           ; And get the result in ECX.

    mov ecx, eax                      ; And use it to calculate the checksum.
    call Checksum

    pop ecx

    ; A zero in AL means that we got clean.
    test al, al
    jnz .EBDANext
    
    mov [BIT.MPS], esi
    jmp .EBDANext

; Move on to the next entry.
.EBDANext:
    ; Move on 16 byte boundaries.
    sub ecx, 0x10
    add esi, 0x10

    test ecx, ecx
    jz .Cont

    ; If ACPI is zero, loop.
    mov eax, [BIT.ACPI]
    test eax, eax
    jz .EBDA

    mov eax, [BIT.MPS]
    test eax, eax
    jz .EBDA

.Cont:
    ; Test the ACPI, MPS and SMBIOS field - if any one of them are zero, try the BIOS area.
    mov eax, [BIT.ACPI]
    test eax, eax
    jz .BIOSPrepare

    mov eax, [BIT.MPS]
    test eax, eax
    jz .BIOSPrepare

    mov eax, [BIT.SMBIOS]
    test eax, eax
    jz .BIOSPrepare

    jmp .Return

; Try the area from 0xE0000 - 0xFFFFF - prepare for it.
.BIOSPrepare:
    ; Start from 0xE0000.
    mov esi, 0xE0000

    ; Check till 0xFFFFF.
    mov ecx, 0x20000
    
; We have the starting address in ESI, and the size to check in ECX.
.BIOS:
; Check if we got the RSDP.
.BIOSRSDP:
    cmp [esi], dword "RSD "
    jne .BIOSMPS

    cmp [esi + 4], dword "PTR "
    jne .BIOSMPS

; Check the checksum - this should pass, else we don't consider it to be valid.
.ChecksumRSDPBIOS:
    push ecx

    ; The RSDP only needs the checksum of the first 20 bytes.
    mov ecx, 20
    call Checksum
   
    pop ecx
   
    ; A zero in AL means that we got clean.
    test al, al
    jnz .BIOSMPS

    mov [BIT.ACPI], esi
    jmp .BIOSNext

.BIOSMPS:    
    cmp [esi], dword "_MP_"
    jne .BIOSSMBIOS

; The checksum of the MPS tables.
.ChecksumMPSBIOS:
    push ecx                          ; Save ECX - since it's used in the Checksum process.

    ; The length is at offset 8 - in 16 byte units.
    movzx eax, byte [esi + 8]
    mov ecx, 0x10                     ; Multiple it by 0x10.
    mul ecx                           ; And get the result in ECX.

    mov ecx, eax                      ; And use it to calculate the checksum.
    call Checksum

    pop ecx

    ; A zero in AL means that we got clean.
    test al, al
    jnz .BIOSNext
    
    mov [BIT.MPS], esi
    jmp .BIOSNext

.BIOSSMBIOS:
    ; If it is _SM_, then it's the SMBIOS table.
    cmp [esi], dword "_SM_"
    jne .BIOSNext

; The checksum of the SMBIOS tables.
.ChecksumSMBIOS:
    push ecx                          ; Saving ECX.

    ; The length is in the 5th index.
    movzx ecx, byte [esi + 5]
    call Checksum                     ; Calculate the checksum.

    pop ecx

    ; Zero = success!
    test al, al
    jnz .BIOSNext
  
    mov [BIT.SMBIOS], esi
    jmp .BIOSNext

; Move on to the next entry.
.BIOSNext:
    ; Move on 16 byte boundaries.
    sub ecx, 0x10
    add esi, 0x10

    ; If we completed all entries - end.
    test ecx, ecx
    jz .Cont2

    ; If ACPI is zero, loop.
    mov eax, [BIT.ACPI]
    test eax, eax
    jz .BIOS

    mov eax, [BIT.MPS]
    test eax, eax
    jz .BIOS

    mov eax, [BIT.SMBIOS]
    test eax, eax
    jz .BIOS

.Cont2:
    ; If MPS is not zero, then return - else try the last kilobyte of lower memory.
    mov eax, [BIT.MPS]
    test eax, eax
    jnz .Return

    ; Get the lower memory into EAX - kilobytes.
    xor eax, eax
    int 0x12

    ; Now multiply it by 0x400.
    mov ecx, 0x400
    mul ecx

    ; Check 0x400 bytes (kilobyte) from the last kilobyte of lower memory.
    sub eax, 0x400
    mov ecx, 0x400
    
; We have the starting address in ESI, and the size to check in ECX.
.Lower:
; Check if we found the MPS tables.
.LowerMPS:    
    cmp [esi], dword "_MP_"
    jne .LowerNext

; The checksum of the MPS tables.
.ChecksumMPSLower:
    push ecx                          ; Save ECX - since it's used in the Checksum process.

    ; The length is at offset 8 - in 16 byte units.
    movzx eax, byte [esi + 8]
    mov ecx, 0x10                     ; Multiple it by 0x10.
    mul ecx                           ; And get the result in ECX.

    mov ecx, eax                      ; And use it to calculate the checksum.
    call Checksum

    pop ecx

    ; A zero in AL means that we got clean.
    test al, al
    jnz .LowerNext
    
    mov [BIT.MPS], esi

; Move on to the next entry.
.LowerNext:
    ; Move on 16 byte boundaries.
    sub ecx, 0x10
    add esi, 0x10

    test ecx, ecx
    jz .Return

    ; If MPS is zero - retry.    
    mov eax, [BIT.MPS]
    test eax, eax
    jz .Lower

.Return:
    popad
    ret

; Calculates the checksum of the given input buffer.
; @esi            The input buffer.
; @ecx            The number of bytes on which to perform the checksum.
;     @rc
;                 Returns the checksum in @al.
Checksum:
    ; Push few registers.
    push esi
    
    ; Clear out AL - in which we would be keeping the checksum.
    xor ax, ax

.Loop:
    ; Add the byte at ESI in AL - and increase the counter.
    add al, [esi]
    inc esi

    ; Decrease the number of bytes left - and check for zero.
    dec ecx
    test ecx, ecx
    jnz .Loop

.Return:
    pop esi
    ret

BITS 16