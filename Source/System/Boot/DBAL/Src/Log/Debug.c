/* Contains structures and definitions for Debug log functions.
* 
*  Copyright (c) 2011 Shikhin Sethi
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation;  either version 3 of the License, or
*  (at your option) any later version.
* 
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
* 
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdint.h>
#include <stdarg.h>
#include <Log.h>
#include <String.h>

// Declare some space for few common things.
uint8_t *VGAFrameBuffer = (uint8_t*)0xB8000;

// Cursor positions would be required to set the hardware cursor, and
// print at the right place.
uint8_t CursorX = 0, CursorY = 0;

// Updates the hardware cursor.
static void SetHardwareCursor()
{
   // Use the saved cursor locations to set the right cursor position.
   uint16_t CursorLinear = (CursorY * 80) + CursorX;
   outb(0x3D4, 14);                   
   outb(0x3D5, (uint8_t)(CursorLinear >> 8));              // The VGA board wants us to first send the higher byte.
   outb(0x3D4, 15);                   
   outb(0x3D5, (uint8_t)CursorLinear);                     // And then the lower byte.
}

// Scrolls the text on the screen up by one line, in text mode.
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

// Print a character 'c' to the screen, in text mode.
// char c                             The character to print.
static void DebugPrintCharText(char C)
{
    // Do some arithmetic on VGAFrameBuffer - where to put.
    uint8_t *Location;

    // Handle all special characters - tabs and newlines, what all I want.
    if(C == '\t')
        CursorX = (CursorX + 8) & ~7;

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

    // Perform some cleanup - by setting the hardware cursor, and scrolling if neccessary.
    SetHardwareCursor();
    ScrollText();
}


// Outputs a null-terminated ASCII string to the monitor.
// char *S                            The null terminated ASCII string.
void DebugPrintStringText(char *S)
{
    // Keep printing characters till we encounter a null character.
    int i = 0;
    while(S[i])
        DebugPrintCharText(S[i++]);
}

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
        C2[i--] = C[j++];
    
    DebugPrintStringText(C2);
}

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
        DebugPrintCharText((Tmp - 0xA) + 'A');
   
    else
        DebugPrintCharText(Tmp + '0');
}

// Prints to the screen, in text mode.
// char *Fmt                          Contains the string to print.
// ...                                And other arguments.
void DebugPrintText(char *Fmt, ...)
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
	
	C = *Fmt++;
	switch(C)
	{
	  case '%':
	    DebugPrintCharText('%');
	    break;
	    
	  case 'd':
	  {
	    // Retrieve the decimal and then print it.
	    uint32_t Decimal = va_arg(List, uint32_t);
	    DebugPrintDecimalText(Decimal);
	    break;
	  }
	  
	  case 'x':
	  {
	    // Retrieve the hexadecimal and then print it.
	    uint32_t Hexadecimal = va_arg(List, uint32_t);
	    DebugPrintHexadecimalText(Hexadecimal);
	    break;
	  }
	  
	  case 's':
	  {
	    // Retrieve the string and then print it.
	    char *String = va_arg(List, char *);
	    DebugPrintStringText(String);
	    break;
	  }
	  
	  case 'c':
	  {
	    // Retieve the character and then print it.
	    uint32_t Char = va_arg(List, uint32_t);
	    DebugPrintCharText((char)Char);
	    break;
	  }
	  
	  default:
	    DebugPrintCharText('X');
	    DebugPrintCharText(C);
	}
    }
    
    va_end(List);
}