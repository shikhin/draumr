 ; Responsible for finding the ACPI, MPS and SMBIOS tables.
 ;
 ; Copyright (c) 2013, Shikhin Sethi
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

SECTION .text

BITS 32

 ; Finds the ACPI, MPS and SMBIOS tables, and stores them in the BIT.
; NOTE: If unable to find any one of them, it lets the address remain to a default 0:0.
TablesFind:
    pushad

    movzx esi, word [0x40E]
    shl esi, 4                        ; Usually, the word at 0x40E contains the address of EBDA shifted right by 4.
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
    movzx ecx, byte [esi + 8]
    shl ecx, 4                        ; Shift left by 4, effectively multiplying by 16
    
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
    add esi, 0x10

    sub ecx, 0x10
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
    movzx ecx, byte [esi + 8]
    shl ecx, 4                         ; Shift left by 4, effectively dividing by 16.
     
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
    add esi, 0x10

    ; If we completed all entries - end.
    sub ecx, 0x10
    jz .Return

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

.Return:
    popad
    ret

 ; Calculates the checksum of the given input buffer.
 ;     ESI -> the input buffer.
 ;     ECX -> the number of bytes on which to perform the checksum.
 ;
 ; Returns:
 ;     AL  -> the checksum.
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
    jnz .Loop

.Return:
    pop esi
    ret

BITS 16