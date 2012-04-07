/* 
 * Contains structures and definitions for Debug log functions.
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

#include <Log.h>
#include <String.h>

// Declare some space for few common things.
uint8_t *VGAFrameBuffer = (uint8_t*)0xB8000;

// Cursor positions would be required to print at the right place.
uint8_t CursorX = 0, CursorY = 0;

/*
 * Scrolls the text on the screen up by one line, in text mode.
 */
static void ScrollText()
{
   // If time has come to scroll (text mode is 80x25), then scroll.
   if(CursorY >= 25)
   {
       // Move everything up a line.
       memmove(VGAFrameBuffer, VGAFrameBuffer + 160, (24 * 80) * 2);

       // Blank the last line - scroll "up".
       for(uint32_t i = (24 * 80) * 2; i < (25 * 80) * 2; i += 2)
       {
           VGAFrameBuffer[i] = ' ';
       }
       // Shift the cursor a line up.
       CursorY = 24;
   }
}

/*
 * Print a character 'c' to the screen, in text mode.
 *     char c -> the character to print.
 */
static void DebugPrintCharText(char C)
{
    // Do some arithmetic on VGAFrameBuffer - where to put.
    uint8_t *Location;

    // Handle all special characters - tabs and newlines, what all I want.
    if(C == '\t')
    {
        CursorX = (CursorX + 8) & ~7;
    }

    else if(C == '\n')
    {
        CursorX = 0;
        CursorY++;
    }
    
    // If it's any other printable character, print it.
    else if(C >= ' ')
    {
        // Get the right location to print at - and then put the character there.
        Location = VGAFrameBuffer + (((CursorY * 80) + CursorX) * 2);
        *Location = C;
        CursorX++;
    }

    // If we have reached the end of the line, insert a new line.
    if(CursorX >= 80)
    {
        CursorX -= 80;                // Move down by 80 characters.
        CursorY++;
    }

    // Perform some cleanup - by scrolling if neccessary.
    ScrollText();
}

/*
 * Outputs a null-terminated ASCII string to the monitor.
 *     char *S -> the null terminated ASCII string.
 */
static void DebugPrintStringText(_CONST char *S)
{
    // Keep printing characters till we encounter a null character.
    int i = 0;
    while(S[i])
        DebugPrintCharText(S[i++]);
}

/*
 * Outputs a decimal to the monitor.
 *     uint32_t N -> the decimal to output.
 */
static void DebugPrintDecimalText(uint32_t N)
{
    // If the number is 0, print <NULL>.
    if(!N)
    {
        DebugPrintStringText("<NULL>");
        return;
    }

    int32_t Temp = N;
    char C[32];
    
    // Get all the numbers as characters starting from the end.
    int i = 0;
    while(Temp > 0)
    {
        C[i] = '0' + (Temp % 10);
        Temp /= 10;
        i++;
    }
    
    C[i] = 0;

    char C2[32];
    C2[i--] = 0;
    
    // Now, since it is reversed, get it straight. And then print it as a string.
    int j = 0;
    while(i >= 0)
    {
        C2[i--] = C[j++];
    }

    DebugPrintStringText(C2);
}

/*
 * Outputs an integer in hexadecimal form to the monitor.
 *     uint32_t N -> the integer to output.
 */
static void DebugPrintHexadecimalText(uint32_t N)
{
    int32_t Tmp;

    DebugPrintStringText("0x");

    char NoZeroes = 1;

    int i;
    for(i = 28; i > 0; i -= 4)
    {
        // Get the 'i'th thingy.
        Tmp = (N >> i) & 0xF;
        if(Tmp == 0 && NoZeroes != 0)
	    DebugPrintCharText('0');
    
        else if(Tmp >= 0xA)
        {
            NoZeroes = 0;
            DebugPrintCharText((Tmp - 0xA) + 'A');
        }
        
        else
        {
            NoZeroes = 0;
            DebugPrintCharText(Tmp + '0');
        }
    }
  
    // And the last thingy left print it if there.
    Tmp = N & 0xF;
    if(Tmp >= 0xA)
    {
        DebugPrintCharText((Tmp - 0xA) + 'A');
    }

    else
    {
        DebugPrintCharText(Tmp + '0');
    }
}

/*
 * Prints to the screen, in text mode.
 *     char *Fmt -> the string containing the format parameters, and the original string.
 *     ...       -> and the rest of the arguments.
 */
void DebugPrintText(_CONST char *Fmt, ...)
{
    // Make a variable arguments list, and start it from Fmt.
    // Yeah, I don't believe in the vsprintf shit.
    // Don't ask why.
    // No, I ain't telling.
    // Sshh.
    va_list List;
    va_start(List, Fmt);
    
    // Store the character temporarily here.
    char C;
    while((C = *Fmt++) != 0)
    {
        if(C != '%')
	    {
	        DebugPrintCharText(C);
	        continue;
	    }
	    
        uint32_t Arg;
        char *StringArg;

	    C = *Fmt++;
	    switch(C)
	    {
	      case '%':
	        DebugPrintCharText('%');
	        break;
	    
	      case 'd':
	        // Retrieve the decimal and then print it.
	        Arg = va_arg(List, uint32_t);
	        DebugPrintDecimalText(Arg);
	        break;
	  
	      case 'x':
	        // Retrieve the hexadecimal and then print it.
	        Arg = va_arg(List, uint32_t);
	        DebugPrintHexadecimalText(Arg);
	        break;
	  
	      case 's':
	        // Retrieve the string and then print it.
	        StringArg = va_arg(List, char *);
	        DebugPrintStringText((_CONST char*)StringArg);
	        break;
	  
	      case 'c':
	        // Retieve the character and then print it.
	        Arg = va_arg(List, uint32_t);
	        DebugPrintCharText((char)Arg);
	        break;
	  
	      default:
	        DebugPrintCharText('X');
	        DebugPrintCharText(C);
	    }
    }
    
    va_end(List);
}