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
        uint32_t Y, Y1, InputIndex, YX1, Y1X, Y1XM1, Y1X1;
        uint8_t Blue, Green, Red;
        uint8_t ERed, EGreen, EBlue;
        for(uint32_t i = 1; i < (BIT.Video.YRes - 1); i++)
        {
            // That's the Y index, the Y + 1 index, and the Y - 1 index.
            Y = i * BIT.Video.XRes;
            Y1 = (i + 1) * BIT.Video.XRes;
            for(uint32_t j = 1; j < (BIT.Video.XRes - 1); j++)
            {
                // Get the input index.
                InputIndex = (Y + j) * 3;
           
                YX1 = (Y + j + 1) * 3;
                Y1X = (Y1 + j) * 3;
                Y1XM1 = (Y1 + j - 1) * 3;
                Y1X1 = (Y1 + j + 1) * 3;
                
                // Get all the colors.
                Blue = Input[InputIndex];
                Green = Input[InputIndex + 1];
                Red = Input[InputIndex + 2];
                
                // Find the Error (F) for each in RGB.
                ERed = Red - (Red & ~0x1F);
                EGreen = Green - (Green & ~0x1F);
                EBlue = Blue - (Blue & ~0x3F);
                
                // Take care of "[x + 1][y]"
                // BLUE.
                uint16_t BlueX1Y = Input[YX1];
                BlueX1Y += (7 * EBlue)/16;
                if(BlueX1Y > 0xFF)
                    BlueX1Y = 0xFF;
                
                Input[YX1] = (uint8_t)BlueX1Y;
                
                // GREEN.
                uint16_t GreenX1Y = Input[YX1 + 1];
                GreenX1Y += (7 * EGreen)/16;
                if(GreenX1Y > 0xFF)
                    GreenX1Y = 0xFF;
                
                Input[YX1 + 1] = (uint8_t)GreenX1Y;
                
                // RED.
                uint16_t RedX1Y = Input[YX1 + 2];
                RedX1Y += (7 * ERed)/16;
                if(RedX1Y > 0xFF)
                    RedX1Y = 0xFF;
                
                Input[YX1 + 2] = (uint8_t)RedX1Y;

                // Take care of "[x - 1][y + 1]"
                // BLUE.
                uint16_t BlueXM1Y1 = Input[Y1XM1];
                BlueXM1Y1 += (3 * EBlue)/16;
                if(BlueXM1Y1 > 0xFF)
                    BlueXM1Y1 = 0xFF;
                
                Input[Y1XM1] = (uint8_t)BlueXM1Y1;
                
                // GREEN.
                uint16_t GreenXM1Y1 = Input[Y1XM1 + 1];
                GreenXM1Y1 += (3 * EGreen)/16;
                if(GreenXM1Y1 > 0xFF)
                    GreenXM1Y1 = 0xFF;
                
                Input[Y1XM1 + 1] = (uint8_t)GreenXM1Y1;
                
                // RED.
                uint16_t RedXM1Y1 = Input[Y1XM1 + 2];
                RedXM1Y1 += (3 * ERed)/16;
                if(RedXM1Y1 > 0xFF)
                    RedXM1Y1 = 0xFF;
                
                Input[Y1XM1 + 2] = (uint8_t)RedXM1Y1;
                
                // Take care of "[x][y + 1]"
                // BLUE.
                uint16_t BlueXY1 = Input[Y1X];
                BlueXY1 += (5 * EBlue)/16;
                if(BlueXY1 > 0xFF)
                    BlueXY1 = 0xFF;
                
                Input[Y1X] = (uint8_t)BlueXY1;
                
                // GREEN.
                uint16_t GreenXY1 = Input[Y1X + 1];
                GreenXY1 += (5 * EGreen)/16;
                if(GreenXY1 > 0xFF)
                    GreenXY1 = 0xFF;
                
                Input[Y1X + 1] = (uint8_t)GreenXY1;
                
                // RED.
                uint16_t RedXY1 = Input[Y1X + 2];
                RedXY1 += (5 * ERed)/16;
                if(RedXY1 > 0xFF)
                    RedXY1 = 0xFF;
                
                Input[Y1X + 2] = (uint8_t)RedXY1;
                
                // Take care of "[x + 1][y + 1]"
                // BLUE.
                uint16_t BlueX1Y1 = Input[Y1X1];
                BlueX1Y1 += EBlue/16;
                if(BlueX1Y1 > 0xFF)
                    BlueX1Y1 = 0xFF;
                
                Input[Y1X1] = (uint8_t)BlueX1Y1;
                
                // GREEN.
                uint16_t GreenX1Y1 = Input[Y1X1 + 1];
                GreenX1Y1 += EGreen/16;
                if(GreenX1Y1 > 0xFF)
                    GreenX1Y1 = 0xFF;
                
                Input[Y1X1 + 1] = (uint8_t)GreenX1Y1;
                
                // RED.
                uint16_t RedX1Y1 = Input[Y1X1 + 2];
                RedX1Y1 += ERed/16;
                if(RedX1Y1 > 0xFF)
                    RedX1Y1 = 0xFF;
                
                Input[Y1X1 + 2] = (uint8_t)RedX1Y1;
                
                Output[Y + j] = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
            }
        }
        
        // Do the bottom and top most layers.
        for(uint32_t i = 0; i < BIT.Video.YRes; i += (BIT.Video.YRes - 1))
        {
            Y = (i * BIT.Video.XRes);
            for(uint32_t j = 0; j < BIT.Video.XRes; j++)
            {
                // Get the input index.
                InputIndex = (Y + j) * 3;
                                
                // Get all the colors.
                Blue = Input[InputIndex];
                Green = Input[InputIndex + 1];
                Red = Input[InputIndex + 2];
                                
                // Get the final converted color.
                uint8_t FinalColor = ConvertColorTo8BPP(Red, Green, Blue);
                                
                Output[Y + j] = FinalColor;
            }
        }
        
        // Do the right and left most columns.
        for(uint32_t i = 0; i < BIT.Video.YRes; i++)
        {
            Y = (i * BIT.Video.XRes);
            for(uint32_t j = 0; j < BIT.Video.XRes; j += (BIT.Video.XRes - 1))
            {
                // Get the input index.
                InputIndex = (Y + j) * 3;
                                                        
                // Get all the colors.
                Blue = Input[InputIndex];
                Green = Input[InputIndex + 1];
                Red = Input[InputIndex + 2];
                                                        
                // Get the final converted color.
                uint8_t FinalColor = ConvertColorTo8BPP(Red, Green, Blue);
                                                       
                Output[Y + j] = FinalColor;
            }
        }
    }
}

