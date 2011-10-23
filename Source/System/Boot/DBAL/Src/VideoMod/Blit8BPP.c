/* Contains definitions to output to the screen, in 8BPP mode.
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
#include <VideoMod/Blit.h>
#include <BIT.h>
#include <String.h>

uint32_t *OldBuffer;

// Blits a buffer of 8bpp to the screen.
// uint32_t *Buffer                   The address of the buffer to blit.
void BlitBuffer8BPP(uint32_t *Buffer)
{
    // Define the Offset, Bit offset, mask, data.
    uint32_t Offset, VideoOffset;                 
    
    VideoOffset = Offset = 0;
	
    // The outerloop for going through the Y axis.
    for(uint32_t i = 0; i < BIT.Video.YRes; i++)
    { 
	// Loop through the X axis for the buffer - blitting the input buffer into a buffer.
        for(uint32_t j = 0; j < BIT.Video.XRes; j += 4, Offset++, VideoOffset++)
	{
	    if(Buffer[Offset] != OldBuffer[Offset])
	    {
	        OldBuffer[Offset] = Buffer[Offset];
		BIT.Video.Address[VideoOffset] = Buffer[Offset];
	    }
	}
	    
	// Skip the bytes between lines thingy.
	VideoOffset += BIT.Video.BytesBetweenLines / 4;
    }
}
