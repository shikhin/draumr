 ; This is a one line description of the contents of this file.
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
 
; NOTES about the file here.
; LIKE - the functions in this file won't actually compile. It's garbage stuff.
; Some people say that coding style should actually be.. um, compilable. I believe 
; it should simply demonstrate the style, not be compilable or something.

; This is a comment, if neccessary, describing the includes.
%include "Mod/SubMod/File.asm"

; This is a comment for a macro, followed by the macro itself - all macros for this file only here.
%define MACRO (1)

; Just some example I copied off the nasm's documentation.
; Hey! Don't blame me. I forgot the format of a multiline macro.
%macro MACRO_2 1
    push ebp
    mov ebp, esp
    sub esp, %1
%endmacro

; This is a comment for some definitions, followed by the definitions themselves.
ModNameVarName:
    dd 0

 ; This is the one line description of the function.
 ; This is the workarounds in the function.
 ;     EAX -> Argument 1.
 ;
 ; Return:
 ;     EBX -> Return value 1.
; NOTE, TODO regarding functions here.
ModNameFuncName:
    ; This is a macro, which does some shit.
    SOME_SHIT

    mov eax, ebx
    mov ecx, edx

    rep movsd

.Return:
    leave
    ret