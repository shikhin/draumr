/* Contains common definitions to output to the screen.
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
#include <OutputMod/Enhance.h>
#include <BIT.h>
#include <PMM.h>
#include <String.h>

// The old buffer, used for comparision to allow unneccessary copying to video memory.
extern uint32_t *OldBuffer;

// The temporary buffer, where we dither. If allocating this fails, dithering is disabled.
extern uint32_t *TempBuffer;

// The temporary buffer, where we keep the error for the next line.
extern uint8_t  *TempErrorLine;

// Gives a buffer, of bpp being what we require, to be blitted to the screen.
// uint32_t *Buffer                   The address of the buffer to print.
void BlitBuffer(uint32_t *Buffer)
{
    // For 8BPP.
    if(BIT.Video.BPP == 8)
    {
        // Dither the buffer, into itself.
        if(!(BIT.Video.VideoFlags & DITHER_DISABLE))
            Dither8BPP((uint8_t*)Buffer, (uint8_t*)TempBuffer);
            
        // Else, just convert - don't dither.
        else
            Convert8BPP((uint8_t*)Buffer, (uint8_t*)TempBuffer);
            
        BlitBuffer8BPP(TempBuffer);
    }
    
    // For 15BPP.
    else if(BIT.Video.BPP == 15)
    {
	    // Convert the buffer into 15bpp.
	    Convert15BPP((uint8_t*)Buffer, (uint8_t*)TempBuffer);
	    BlitBuffer15BPP(TempBuffer);
	}
}

