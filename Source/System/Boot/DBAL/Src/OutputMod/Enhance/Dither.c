/* Contains common definitions to dither images and stuff.
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
#include <String.h>
#include <BIT.h>
#include <OutputMod/Enhance.h>

// Converts a 24bpp pixel to 8bpp pixel - through some intense calculation.
// uint8_t Red                        The red part of the pixel.
// uint8_t Green                      The green part of the pixel.
// uint8_t Blue                       The blue part of the pixel.
//     rc
//                                    The 8bpp pixel color is returned.
static uint8_t ConvertColorTo8BPP(uint8_t Red, uint8_t Green, uint8_t Blue)
{
    uint8_t Color = 0;
    
    // Round it off to the nearest thingy.
    Red &= ~0x1F;
    Green &= ~0x1F;
    Blue &= ~0x3F;
    
    // Find the top most 3 bits.
    Color |= (Red) | (Green >> 3) | (Blue >> 6);
        
    return Color;
}

// Converts a buffer to the required BPP format, INTO the DrawBoard - and dithers if required too.
// uint8_t  *Input                    The input buffer, which we are about to convert and/or dither.
// uint8_t  *Output                   The output buffer, where we will store the converted thingy.
// NOTE: The Input & Output buffer would have a size of BIT.Video.XRes * BIT.Video.YRes
void Dither(uint8_t *Input, uint8_t *Output)
{
    // If we don't need to convert, do a quick memcpy, and return.
    if(BIT.Video.BPP == 24)
    {
        memcpy(Output, Input, (BIT.Video.XRes * BIT.Video.YRes * BIT.Video.BPP) / 8);
    }
    
    // Else if, the output format is 4BPP.
    else if(BIT.Video.BPP == 4)
    {
        // TODO    
    }    
    
    // Else if, it is 8BPP, do the conversion.
    else if(BIT.Video.BPP == 8)
    {
        // SIF images are in BMP format - inverted according to rows.
        uint32_t InputBufferIndex = 0;
        for(int32_t i = (BIT.Video.YRes - 1); i >= 0; i--)
            for(uint32_t j = 0; j < BIT.Video.XRes; j++)
            {
                // Get all the colors.
                uint8_t Red = Input[InputBufferIndex++];
                uint8_t Green = Input[InputBufferIndex++];
                uint8_t Blue = Input[InputBufferIndex++];
                
                // Get the final converted color.
                uint8_t FinalColor = ConvertColorTo8BPP(Red, Green, Blue);
            
                Output[(i * BIT.Video.XRes) + j] = FinalColor;
            }
    }
}

