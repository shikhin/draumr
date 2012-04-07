/*
 * Contains functions to abort boot.
 *
 * Copyright (c) 2012, Shikhin Sethi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Draumr nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Abort.h>
#include <Log.h>
#include <String.h>
 
/*
 * Aborts boot, by giving a endless beep, and trying to print a message on the screen.
 *     _CONST char *String -> the message to print
 *
 * Returns:
 *     Boot                -> halts the processor.
 */
void AbortBoot(_CONST char *String)
{
    DebugPrintText(String);           // Print the message using the text version. 

    outb(0x42, 0xB6);
    outb(0x42, 0xD1);                 // Send lower 16-bits of count for frequency to play.            
    outb(0x42, 0x11);                 // Send higher 16-bits of count for frequency to play.
       
    outb(0x61, (inb(0x61) | 0x3));    // Set the Speaker enable and other required bit - SPEAK. 

    // And halt forever in the dark shadowy land of Motherboardy.
    for(;;)
        __asm__ __volatile__("hlt");
}