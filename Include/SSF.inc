 ; Defines some macros for the SSF file header.
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

; The SSF file header.
struc SFS_fileHeader
    ident:      resd 1
    fileIdent:  resd 1
    archIdent:  resw 1
    startAddr:  resq 1
    codeLen:    resd 1
    dataLen:    resd 1
    bssLen:     resd 1
    entryPoint: resq 1
    CRC32:      resd 1
endstruc

; The SSF file header identifier.
%define SSF_IDENT       "SSF0"

; File identifiers.
%define STAGE_15_IDENT  0x00000001
%define BAL_IDENT       0x00000002
%define LOADER_IDENT    0x00000003
%define KERNEL_IDENT    0x00000004
%define PMM_IDENT       0x00000005
%define VMM_IDENT       0x00000006

; Architecture identifiers.
%define X86_IDENT       0x0001
%define X86_PAE_IDENT   0x0002
%define X86_64_IDENT    0x0003