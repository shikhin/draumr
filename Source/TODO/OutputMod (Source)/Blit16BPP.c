/* Contains definitions to output to the screen, in 16BPP mode.
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
#include <OutputMod/Blit.h>
#include <BIT.h>
#include <String.h>

// The old buffer, used for comparision to allow unneccessary copying to video memory.
extern uint32_t *OldBuffer;

// The temporary buffer, where we keep the error for the next line.
extern uint8_t  *TempErrorLine;

// Blits a buffer of 16bpp to the screen.
// uint32_t *Buffer                   The address of the buffer to blit.
void BlitBuffer16BPP(uint32_t *Buffer)
{
	if(OldBuffer)
	{
		uint8_t  *BufferP = (uint8_t*)Buffer;
        // Define the Video Address and the Old Buffer address (temp).
        uint32_t *VidAddress = BIT.Video.Address,
                 *OldBufferTemp = OldBuffer;
	
        uint32_t DwordsBetweenLines = BIT.Video.BytesBetweenLines / 4, Value;
        // The outerloop for going through the Y axis.
        for(uint32_t i = 0; i < BIT.Video.YRes; i++, VidAddress += DwordsBetweenLines)
        {
			 // Loop through the X axis for the buffer - blitting the input buffer into a buffer.
            for(uint32_t j = 0; j < BIT.Video.XRes; j += 2, OldBufferTemp++,
                                                            VidAddress++)
	        {  
				Value  = ((*BufferP++ & ~0x7) >> 3);
				Value |= ((*BufferP++ & ~0x3) << 3);
				Value |= ((*BufferP++ & ~0x7) << 8);
				           
				Value |= ((*BufferP++ & ~0x7) >> 3) << 16;
				Value |= ((*BufferP++ & ~0x3) << 3) << 16;
				Value |= ((*BufferP++ & ~0x7) << 8) << 16;
				      
	            if(Value != *OldBufferTemp)
	            {
	                *OldBufferTemp = Value;
		            *VidAddress = Value;
	            }
	        }
	    }
    }
    
    else
    {
		// Define the Video Address and the Old Buffer address (temp).
        uint32_t *VidAddress = BIT.Video.Address;
        uint16_t *TempLine;
        uint8_t  *BufferP = (uint8_t*)Buffer;
        
        uint32_t DwordsBetweenLines = BIT.Video.BytesBetweenLines / 4;
        uint32_t XRes = BIT.Video.XRes / 2, XRes1 = BIT.Video.XRes * 2;
        
        uint32_t Blue, Green, Red;
        
        // The outerloop for going through the Y axis.
        for(uint32_t i = 0; i < BIT.Video.YRes; i++, VidAddress += DwordsBetweenLines)
        {		
			TempLine = (uint16_t*)TempErrorLine;
            for(uint32_t j = 0; j < BIT.Video.XRes; j++)
            {
                // Get all the colors.
                Blue = *BufferP++;
                Green = *BufferP++;
                Red = *BufferP++;
          
                // And output the required color.
                *TempLine++ = ((Red & ~0x7) << 8) | 
                               ((Green & ~0x3) << 3) | ((Blue & ~0x7) >> 3);
            }

			memcpy(VidAddress, TempErrorLine, XRes1);
			VidAddress += XRes;
        }
    }
}
