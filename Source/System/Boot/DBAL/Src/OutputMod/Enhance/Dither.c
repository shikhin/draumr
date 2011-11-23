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

// Define the RGB for the 16 colors in the 4bpp mode.
uint32_t RGB4[16] = 
{
    0x000000, 0x0000AA,
    0x00AA00, 0x00AAAA,
    0xAA0000, 0xAA00AA,
    0xAA5500, 0xAAAAAA,
    0x555555, 0x5555FF,
    0x55FF55, 0x55FFFF,
    0xFF5555, 0xFF55FF,
    0xFFFF55, 0xFFFFFF
};

// Converts a 24bpp pixel to 4bpp pixel - through some intense calculation.
// uint8_t Red                        The red part of the pixel.
// uint8_t Green                      The green part of the pixel.
// uint8_t Blue                       The blue part of the pixel.
//     rc
//                                    The 4bpp pixel color is returned.
static uint8_t ConvertColorTo4BPP(uint8_t Red, uint8_t Green, uint8_t Blue)
{
    uint32_t Score;
    uint32_t LowestScore = ~0;
    uint8_t Color = 0, i;
    
    for(i = 0; i < 16; i++)
    {
        int32_t RedScore = Red - ((RGB4[i] & 0xFF0000) >> 16);      
        int32_t GreenScore = Green - ((RGB4[i] & 0x00FF00) >> 8);
        int32_t BlueScore = Blue - ((RGB4[i] & 0x0000FF));
        
        if(RedScore < 0)
            RedScore *= -1;
        
        if(GreenScore < 0)
            GreenScore *= -1;
        
        if(BlueScore < 0)
            BlueScore *= -1;
        
        Score = RedScore + GreenScore + BlueScore;
        if(Score < LowestScore)
        {
            LowestScore = Score;
            Color = i;
        }
    }
    
    return Color;
}

// Taking the error, and few other variables - spreads the error arround the current pixel, doing floyd-steinberg dithering.
// uint8_t *Input                     The pointer to the image we are dithering.
// uint32_t j                         The current 'x' - in the input buffer.
// uint32_t Y                         The current 'y' multiplied by the 'x' axis to get a index - just for some pre-calculations.
// uint32_t Y1                        The current 'y' + 1 multiplied by the 'x' axis.
// int16_t  ERed        
// int16_t  EGreen                    THE ERROR OF RED, GREEN AND BLUE COLORS.
// int16_t  EBlue
static void DitherForPixel(uint8_t *Input, uint32_t j, uint32_t Y, uint32_t Y1,
                           int16_t ERed, int16_t EGreen, int16_t EBlue)
{
    // NOTE: Ok, so the naming convention here, and the next function might seem weird (for variables).
    // But, I decided on it to keep it short, as weel as meaninful.
    // A simple X or Y in capital, followed by another X/Y in capital (not any number or something) means
    // that it is simply the i/j index multiplied by all that it needs to be multiplied to (XRes, 3 (for 24bpp)).
    // A X/Y in capital, followed by a number, means that THAT number is added to X/Y and then multiplied
    // by whatever it needs to be multiplied to (read above).
    // A X/Y in capital, followed by a M and then a number, means that the number is subtracted, and then multiplied.
    
    // So that's it. Hope I was clear :P
    uint32_t YX1, Y1X, Y1XM1, Y1X1;
    YX1 = (Y + j + 1) * 3;
    Y1X = (Y1 + j) * 3;
    Y1XM1 = (Y1 + j - 1) * 3;
    Y1X1 = (Y1 + j + 1) * 3;
    
    // Take care of "[x + 1][y]"
    // BLUE.
    int16_t BlueX1Y = Input[YX1];
    BlueX1Y += (7 * EBlue)/16;
    if(BlueX1Y > 0xFF)
        BlueX1Y = 0xFF;
    
    else if(BlueX1Y < 0)
        BlueX1Y = 0x00;
    
    Input[YX1] = (uint8_t)BlueX1Y;
    
    // GREEN.
    int16_t GreenX1Y = Input[YX1 + 1];
    GreenX1Y += (7 * EGreen)/16;
    if(GreenX1Y > 0xFF)
        GreenX1Y = 0xFF;
    
    else if(GreenX1Y < 0)
        GreenX1Y = 0x00;
    
    Input[YX1 + 1] = (uint8_t)GreenX1Y;
    
    // RED.
    int16_t RedX1Y = Input[YX1 + 2];
    RedX1Y += (7 * ERed)/16;
    if(RedX1Y > 0xFF)
        RedX1Y = 0xFF;
    
    else if(RedX1Y < 0x00)
        RedX1Y = 0x00;
    
    Input[YX1 + 2] = (uint8_t)RedX1Y;
    
    // Take care of "[x - 1][y + 1]"
    // BLUE.
    int16_t BlueXM1Y1 = Input[Y1XM1];
    BlueXM1Y1 += (3 * EBlue)/16;
    if(BlueXM1Y1 > 0xFF)
        BlueXM1Y1 = 0xFF;
    
    else if(BlueXM1Y1 < 0x00)
        BlueXM1Y1 = 0x00;
    
    Input[Y1XM1] = (uint8_t)BlueXM1Y1;
    
    // GREEN.
    int16_t GreenXM1Y1 = Input[Y1XM1 + 1];
    GreenXM1Y1 += (3 * EGreen)/16;
    if(GreenXM1Y1 > 0xFF)
        GreenXM1Y1 = 0xFF;
    
    else if(GreenXM1Y1 < 0x00)
        GreenXM1Y1 = 0x00;
    
    Input[Y1XM1 + 1] = (uint8_t)GreenXM1Y1;
    
    // RED.
    int16_t RedXM1Y1 = Input[Y1XM1 + 2];
    RedXM1Y1 += (3 * ERed)/16;
    if(RedXM1Y1 > 0xFF)
        RedXM1Y1 = 0xFF;
    
    else if(RedXM1Y1 < 0x00)
        RedXM1Y1 = 0x00;
    
    Input[Y1XM1 + 2] = (uint8_t)RedXM1Y1;
    
    // Take care of "[x][y + 1]"
    // BLUE.
    int16_t BlueXY1 = Input[Y1X];
    BlueXY1 += (5 * EBlue)/16;
    if(BlueXY1 > 0xFF)
        BlueXY1 = 0xFF;
    
    else if(BlueXY1 < 0x00)
        BlueXY1 = 0x00;
    
    Input[Y1X] = (uint8_t)BlueXY1;
    
    // GREEN.
    int16_t GreenXY1 = Input[Y1X + 1];
    GreenXY1 += (5 * EGreen)/16;
    if(GreenXY1 > 0xFF)
        GreenXY1 = 0xFF;
    
    else if(GreenXY1 < 0x00)
        GreenXY1 = 0x00;
    
    Input[Y1X + 1] = (uint8_t)GreenXY1;
    
    // RED.
    int16_t RedXY1 = Input[Y1X + 2];
    RedXY1 += (5 * ERed)/16;
    if(RedXY1 > 0xFF)
        RedXY1 = 0xFF;
    
    else if(RedXY1 < 0x00)
        RedXY1 = 0x00;
    
    Input[Y1X + 2] = (uint8_t)RedXY1;
    
    // Take care of "[x + 1][y + 1]"
    // BLUE.
    int16_t BlueX1Y1 = Input[Y1X1];
    BlueX1Y1 += EBlue/16;
    if(BlueX1Y1 > 0xFF)
        BlueX1Y1 = 0xFF;
    
    else if(BlueX1Y1 < 0x00)
        BlueX1Y1 = 0x00;
    
    Input[Y1X1] = (uint8_t)BlueX1Y1;
    
    // GREEN.
    int16_t GreenX1Y1 = Input[Y1X1 + 1];
    GreenX1Y1 += EGreen/16;
    if(GreenX1Y1 > 0xFF)
        GreenX1Y1 = 0xFF;
    
    else if(GreenX1Y1 < 0x00)
        GreenX1Y1 = 0x00;
    
    Input[Y1X1 + 1] = (uint8_t)GreenX1Y1;
    
    // RED.
    int16_t RedX1Y1 = Input[Y1X1 + 2];
    RedX1Y1 += ERed/16;
    if(RedX1Y1 > 0xFF)
        RedX1Y1 = 0xFF;
    
    else if(RedX1Y1 < 0x00)
        RedX1Y1 = 0x00;
    
    Input[Y1X1 + 2] = (uint8_t)RedX1Y1;                                    
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
        uint32_t Y, Y1, InputIndex;
        uint8_t Blue, Green, Red;
        int16_t ERed, EGreen, EBlue;
        for(uint32_t i = 1; i < (BIT.Video.YRes - 1); i++)
        {
            // That's the Y index, the Y + 1 index, and the Y - 1 index.
            Y = i * BIT.Video.XRes;
            Y1 = Y + BIT.Video.XRes;
            for(uint32_t j = 1; j < (BIT.Video.XRes - 1); j++)
            {
                // Get the input index.
                InputIndex = (Y + j) * 3;
           
                // Get all the colors.
                Blue = Input[InputIndex];
                Green = Input[InputIndex + 1];
                Red = Input[InputIndex + 2];
                
                // Get the final color.
                uint8_t FinalPixel = ConvertColorTo4BPP(Red, Green, Blue);
                
                ERed = (int16_t)Red - (int16_t)((RGB4[FinalPixel] & 0xFF0000) >> 16);
                EGreen = (int16_t)Green - (int16_t)((RGB4[FinalPixel] & 0x00FF00) >> 8);
                EBlue = (int16_t)Blue - (int16_t)((RGB4[FinalPixel] & 0x0000FF));
                
                // Dither for the particular pixel.
                DitherForPixel(Input, j, Y, Y1, ERed, EGreen, EBlue);                
                
                // And output the required color.
                uint32_t ByteOffset = (j + (i * BIT.Video.XRes)) / 8;
                uint32_t BitOffset = 7 - ((j + (i * BIT.Video.XRes)) % 8);
                
                Output[ByteOffset] |= (FinalPixel & 1) << BitOffset;
                Output[PLANE_OFFSET(1) + ByteOffset] |= ((FinalPixel & 2) >> 1) << BitOffset; 
                Output[PLANE_OFFSET(2) + ByteOffset] |= ((FinalPixel & 4) >> 2) << BitOffset;
                Output[PLANE_OFFSET(3) + ByteOffset] |= ((FinalPixel & 8) >> 3) << BitOffset;
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
                uint8_t FinalPixel = ConvertColorTo4BPP(Red, Green, Blue);
                                
                // And output the required color.
                uint32_t ByteOffset = (j + (i * BIT.Video.XRes)) / 8;
                uint32_t BitOffset = 7 - ((j + (i * BIT.Video.XRes)) % 8);
                
                Output[ByteOffset] |= (FinalPixel & 1) << BitOffset;
                Output[PLANE_OFFSET(1) + ByteOffset] |= ((FinalPixel & 2) >> 1) << BitOffset; 
                Output[PLANE_OFFSET(2) + ByteOffset] |= ((FinalPixel & 4) >> 2) << BitOffset;
                Output[PLANE_OFFSET(3) + ByteOffset] |= ((FinalPixel & 8) >> 3) << BitOffset;
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
                uint8_t FinalPixel = ConvertColorTo4BPP(Red, Green, Blue);
                                           
                // And output the required color.
                uint32_t ByteOffset = (j + (i * BIT.Video.XRes)) / 8;
                uint32_t BitOffset = 7 - ((j + (i * BIT.Video.XRes)) % 8);
                
                Output[ByteOffset] |= (FinalPixel & 1) << BitOffset;
                Output[PLANE_OFFSET(1) + ByteOffset] |= ((FinalPixel & 2) >> 1) << BitOffset; 
                Output[PLANE_OFFSET(2) + ByteOffset] |= ((FinalPixel & 4) >> 2) << BitOffset;
                Output[PLANE_OFFSET(3) + ByteOffset] |= ((FinalPixel & 8) >> 3) << BitOffset;
            }
        }        
    }    
    
    // Else if, it is 8BPP, do the conversion.
    else if(BIT.Video.BPP == 8)
    {
        uint32_t Y, Y1, InputIndex;
        uint8_t Blue, Green, Red;
        uint8_t ERed, EGreen, EBlue;
        for(uint32_t i = 1; i < (BIT.Video.YRes - 1); i++)
        {
            // That's the Y index, the Y + 1 index, and the Y - 1 index.
            Y = i * BIT.Video.XRes;
            Y1 = Y + BIT.Video.XRes;
            for(uint32_t j = 1; j < (BIT.Video.XRes - 1); j++)
            {
                // Get the input index.
                InputIndex = (Y + j) * 3;
           
                // Get all the colors.
                Blue = Input[InputIndex];
                Green = Input[InputIndex + 1];
                Red = Input[InputIndex + 2];
                
                // Find the Error (F) for each in RGB.
                ERed = Red - (Red & ~0x1F);
                EGreen = Green - (Green & ~0x1F);
                EBlue = Blue - (Blue & ~0x3F);
                
                // Dither for the particular pixel.
                DitherForPixel(Input, j, Y, Y1, (int16_t)ERed, (int16_t)EGreen, (int16_t)EBlue);                
                
                // And output the required color.
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

