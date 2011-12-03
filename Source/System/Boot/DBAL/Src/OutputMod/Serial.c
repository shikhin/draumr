/* Contains common Serial port definitions.
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
#include <OutputMod/Serial.h>
#include <String.h>
#include <stdarg.h>
#include <BIT.h>

// Outputs a character 'c' via the serial port connection.
// char c                             The character to output.
static void OutputSerialChar(char C)
{
    if(!(BIT.Serial.SerialFlags & SERIAL_USED))
        return;
    
    uint32_t Timeout = 0;
    // Read from the Line status register.
    // And keep looping till the transmit register isn't empty.
    // And, till Timeout < MIN_TIMEOUT. If we reach MIN_TIMEOUT first, then faulty port.
    while((!(inb(BIT.Serial.Port + 5) & 0x20))
          && (Timeout < MIN_TIMEOUT))
    {
        __asm__ __volatile__("pause");
        Timeout++;
    }
  
    // If we reached timeout, then assume somebody plugged it out, and assume no serial port present.
    if(Timeout == MIN_TIMEOUT)
    {
        BIT.Serial.SerialFlags &= ~SERIAL_USED;
        return;
    }
    
    // Output C.
    outb(BIT.Serial.Port, C);
}


// Outputs a null-terminated ASCII string via the serial port connection.
// char *S                            The null terminated ASCII string.
void OutputSerialString(char *S)
{
    if(!(BIT.Serial.SerialFlags & SERIAL_USED))
        return;
    
    // Keep outputting characters till we encounter a null character.
    int i = 0;
    while(S[i])
        OutputSerialChar(S[i++]);
}

static void OutputSerialDecimal(uint32_t N)
{
    // If the number is 0, output <NULL>.
    if(!N)
    {
        OutputSerialString("<NULL>");
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
    
    // Now, since it is reversed, get it straight. And then output it as a string.
    int j = 0;
    while(i >= 0)
        C2[i--] = C[j++];
    
    OutputSerialString(C2);
}

static void OutputSerialHexadecimal(uint32_t N)
{
    int32_t Tmp;

    OutputSerialString("0x");

    char NoZeroes = 1;

    int i;
    for(i = 28; i > 0; i -= 4)
    {
        // Get the 'i'th thingy.
        Tmp = (N >> i) & 0xF;
        if(Tmp == 0 && NoZeroes != 0)
	    OutputSerialChar('0');
    
        else if(Tmp >= 0xA)
        {
            NoZeroes = 0;
            OutputSerialChar((Tmp - 0xA) + 'A');
        }
        
        else
        {
            NoZeroes = 0;
            OutputSerialChar(Tmp + '0');
        }
    }
  
    // And the last thingy left output it if there.
    Tmp = N & 0xF;
    if(Tmp >= 0xA)
        OutputSerialChar((Tmp - 0xA) + 'A');
   
    else
        OutputSerialChar(Tmp + '0');
}

// Outputs some text to the screen, via the serial port interface.
// char *Fmt                          Contains the string to output.
// ...                                And other arguments.
void OutputSerial(char *Fmt, ...)
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
	    OutputSerialChar(C);
	    continue;
	}
	
	C = *Fmt++;
	switch(C)
	{
	  case '%':
	    OutputSerialChar('%');
	    break;
	    
	  case 'd':
	  {
	    // Retrieve the decimal and then output it.
	    uint32_t Decimal = va_arg(List, uint32_t);
	    OutputSerialDecimal(Decimal);
	    break;
	  }
	  
	  case 'x':
	  {
	    // Retrieve the hexadecimal and then output it.
	    uint32_t Hexadecimal = va_arg(List, uint32_t);
	    OutputSerialHexadecimal(Hexadecimal);
	    break;
	  }
	  
	  case 's':
	  {
	    // Retrieve the string and then output it.
	    char *String = va_arg(List, char *);
	    OutputSerialString(String);
	    break;
	  }
	  
	  case 'c':
	  {
	    // Retieve the character and then output it.
	    uint32_t Char = va_arg(List, uint32_t);
	    OutputSerialChar((char)Char);
	    break;
	  }
	  
	  default:
	    OutputSerialChar('X');
	    OutputSerialChar(C);
	}
    }
    
    va_end(List);
}