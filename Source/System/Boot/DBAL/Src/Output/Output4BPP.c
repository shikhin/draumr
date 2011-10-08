/* Contains definitions to output to the screen, in 4BPP mode.
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
#include <Output/Output.h>
#include <BIT.h>
#include <String.h>
#include <Log.h>

extern uint32_t CurrentX, CurrentY;

// Prints a buffer of 4bpp to the screen.
// uint8_t *Buffer                    The address of the buffer to print.
// uint32_t X                         The X size for the buffer.
// uint32_t Y                         The Y size for the buffer.
void BufferOutput4BPP(uint8_t *Buffer, uint32_t X, uint32_t Y)
{
    // Use ScreenX and ScreenY for all arithmetic later.
    uint32_t ScreenX = CurrentX;
    uint32_t ScreenY = CurrentY;
    
    // Define the Offset, Bit offset, mask, data.
    uint32_t Offset, Bit, Mask, Data;
    uint8_t PlaneBit, PlaneShift;
    
    // Set the address register to a) The Write Map Select Register b) The Read Map Select Register.
    outb(0x03C4, 0x02);
    outb(0x03CE, 0x04);
    
    for(PlaneBit = 1, PlaneShift = 0; 
	PlaneBit <= 8; 
        PlaneBit <<= 1, PlaneShift++) 
    { 
        outb(0x03C5, PlaneBit); 
	outb(0x03CF, PlaneShift);
        // The outerloop for going through the Y axis - reset X at the end of every loop, while increasing Y.
        for(uint32_t i = 0; i < Y; i++, ScreenX = CurrentX, ScreenY++)
        {
            // Loop through the X axis for the buffer.
            for(uint32_t j = 0; j < X; j++, ScreenX++)
	    {
	        // Calculate the Offset and the bit from the starting of the display uffer.
	        // The offset is equal to:
                Offset = ((ScreenX +                                                           // The X axis.
                          (ScreenY * BIT.Video.XRes)) / 8) +                                   // Y * X Resolution of screen, get in bytes.
		          (ScreenY * BIT.Video.BytesBetweenLines);                             // And the extra bytes between lines.
                Bit = 7 - ((ScreenX +
                           (ScreenY * BIT.Video.XRes) + 
		           (ScreenY * (BIT.Video.BytesBetweenLines * 8))) & 7);                // Get the bit offset.
                Mask = 0xFF & ~(0x01 << Bit);                                                   // And then, the mask.
	       
	        Data = BIT.Video.Address[Offset] & Mask;                                       // Get the data at that area, so that we don't destroy it.
	        BIT.Video.Address[Offset] = Data | (((Buffer[j + (i * X)] >> PlaneShift) & 1) << Bit);    // And finally, write t that address.
	    }
        }
        
        // Reset ScreenX and ScreenY for next plane.
        ScreenX = CurrentX;
	ScreenY = CurrentY;
    }
    
    // Move the current x forward.
    CurrentX += X;
    
    // If we have crossed the resolution of the monitor, move on to next Y.
    if(CurrentX >= BIT.Video.XRes)
    {
        // Move X back and Y up.
        CurrentX -= BIT.Video.XRes;
	CurrentY += Y;
    }
}