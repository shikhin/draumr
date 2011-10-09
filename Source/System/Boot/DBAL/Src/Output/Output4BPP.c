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
uint32_t TimestampA, TimestampB;

// Prints a buffer of 4bpp to the screen.
// uint8_t *Buffer                    The address of the buffer to print.
// uint32_t X                         The X size for the buffer.
// uint32_t Y                         The Y size for the buffer.
void BufferOutput4BPP(uint8_t *Buffer, uint32_t X, uint32_t Y)
{
    __asm__ __volatile__("cpuid" ::: "%eax", "%ebx", "%ecx", "%edx");
    __asm__ __volatile__("rdtsc" : "=a"(TimestampA) :: "%edx");

    // Define the Offset, Bit offset, mask, data.
    uint32_t Offset, BufferOffset, WriteData, TempOffset;                 
    uint8_t Bit, PlaneBit, PlaneShift;
    uint8_t *TempAddress;
   
    for(PlaneBit = 1, PlaneShift = 0; 
	PlaneBit <= 8; 
        PlaneBit <<= 1, PlaneShift++) 
    {    
        // Calculate the Offset from the starting of the display buffer.
        // The offset is equal to:
        Offset = ((CurrentX +                                                              // The X axis.
                  (CurrentY * BIT.Video.XRes)) / 32) +                                     // Y * X Resolution of screen, get in bytes.
	          ((CurrentY * BIT.Video.BytesBetweenLines) / 4);                          // And the extra bytes between lines.
    
        outb(0x03C5, PlaneBit); 
	outb(0x03CF, PlaneShift);
     
	// The outerloop for going through the Y axis.
        for(uint32_t i = 0; i < Y; i++, Offset += (BIT.Video.XRes) / 32)
        { 
	    uint32_t j = 0;
	    
	    // Do till we reach a 32-bit boundary on the video buffer.
	    for(j = 0, BufferOffset = j + (i * X);
		(j < X) && (j < (CurrentX % 32));
		j++, BufferOffset++)
	    {
	        // If WriteData is empty, loop through this one - never overwrite things.
	        WriteData = ((Buffer[BufferOffset] >> PlaneShift) & 1);
		if(!WriteData)
		    continue;
		
		Bit = 31 - (j % 32);                                                           // Get the bit offset.

		BIT.Video.Address[Offset] |= (WriteData << Bit);                               // And finally, write t that address.
	    }
	    
	    if(j > 0)
	        Offset++;
            
	    // Don't trash the original offset.
	    TempOffset = Offset;
            // Do all 32-bit things, so we don't have to read from the video memory.
            for(BufferOffset = j + (i * X);
		(j + 32) <= X;
	        j += 32, BufferOffset += 32, TempOffset++)
	    {
	        WriteData = (Buffer[BufferOffset] >> PlaneShift) & 1;
	        for(uint32_t k = 1; k < 31; k += 5)
		{
		    // Unroll the loop a bit - improves performance.
		    WriteData <<= 5;
		    WriteData |= ((Buffer[BufferOffset + k] >> PlaneShift) & 1);
		    WriteData |= ((Buffer[BufferOffset + k + 1] >> PlaneShift) & 1) << 1;
		    WriteData |= ((Buffer[BufferOffset + k + 2] >> PlaneShift) & 1) << 2;
		    WriteData |= ((Buffer[BufferOffset + k + 3] >> PlaneShift) & 1) << 3;
		    WriteData |= ((Buffer[BufferOffset + k + 4] >> PlaneShift) & 1) << 4;
		}
		WriteData = (WriteData << 1) | ((Buffer[BufferOffset + 31] >> PlaneShift) & 1);
		
		if(!WriteData)
		    continue;
		
		// And simply write the WriteData.
	        BIT.Video.Address[TempOffset] = WriteData; 
	    }
            
	    // Don't trash the original offset.
	    TempOffset = Offset * 4;
	    TempAddress = (uint8_t*)BIT.Video.Address;
            // Do all 8-bit things, so we don't have to read from the video memory.
            for(BufferOffset = j + (i * X);
		(j + 8) <= X;
	        j += 8, BufferOffset += 8, TempOffset++)
	    {
	        WriteData = (Buffer[BufferOffset] >> PlaneShift) & 1;
	        for(uint32_t k = 1; k < 7; k += 2)
		{
		    WriteData <<= 2;
		    WriteData |= ((Buffer[BufferOffset + k] >> PlaneShift) & 1);
		    WriteData |= ((Buffer[BufferOffset + k + 1] >> PlaneShift) & 1);
		}
		WriteData = (WriteData << 1) | ((Buffer[BufferOffset + 7] >> PlaneShift) & 1);
		
		if(!WriteData)
		    continue;
		
		// And simply write the WriteData at the correct byte.
		TempAddress[TempOffset] = (uint8_t)WriteData; 
	    }
            
            // Loop through the X axis for the buffer - finishing the last left bits.
            for(BufferOffset = j + (i * X); j < X; j++, BufferOffset++)
	    {
	        // If WriteData is a zero, loop through it. We assume that we never overwrite things.
	        WriteData = ((Buffer[BufferOffset] >> PlaneShift) & 1);
		if(!WriteData)
		    continue;
		
                Bit = 31 - (j % 32);                                                           // Get the bit offset.
                
		BIT.Video.Address[Offset] |= (WriteData << Bit);                               // And finally, write to that address.
	    }
        }
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
    __asm__ __volatile__("rdtsc" : "=a"(TimestampB) :: "%edx");
}