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
static uint8_t ConvertTo4BPP(uint8_t Red, uint8_t Green, uint8_t Blue)
{
    uint32_t Score;
    int32_t LowestScore = -1;
    uint8_t Color = 0, i;
    
    for(i = 0; i < 16; i++)
    {
        int32_t RedScore = Red - ((RGB4[i] & 0xFF0000) >> 16);      
        int32_t GreenScore = Green - ((RGB4[i] & 0x00FF00) >> 8);
        int32_t BlueScore = Blue - ((RGB4[i] & 0x0000FF));
        
        if(RedScore < 0x00)
            RedScore *= -1;
        
        if(GreenScore < 0x00)
            GreenScore *= -1;
        
        if(BlueScore < 0x00)
            BlueScore *= -1;
        
        Score = RedScore + GreenScore + BlueScore;
        if(Score < (uint32_t)LowestScore)
        {
            LowestScore = Score;
            Color = i;
        }
    }
    
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
        uint32_t InputIndex;
        int32_t Blue, Green, Red;
        // Temporary red, green and blue.
        int32_t TRed, TGreen, TBlue;
        uint32_t ByteOffset, BitOffset;
        
        // NOTE: Ok, so the naming convention here, and the next function might seem weird (for variables).
        // But, I decided on it to keep it short, as well as meaningful.
        // A simple X or Y in capital, followed by another X/Y in capital (not any number or something) means
        // that it is simply the i/j index multiplied by all that it needs to be multiplied to (XRes, 3 bytes per pixel (for 24bpp)).
        // A X/Y in capital, followed by a number, means that THAT number is added to X/Y and then multiplied
        // by whatever it needs to be multiplied to (read above).
        // A X/Y in capital, followed by a M and then a number, means that the number is subtracted, and then multiplied.
        
        // So that's it. Hope I was clear :P
        uint32_t Y1X = (BIT.Video.XRes * 3), Y1XM1 = (Y1X - 3), Y1X1 = (Y1X + 3), IXRes;
        
        IXRes = InputIndex = 0;
        
        for(uint32_t i = 0; i < (BIT.Video.YRes - 1); i++, IXRes += BIT.Video.XRes) 
        {
            // Get all the colors.
            Blue = Input[InputIndex];
            Green = Input[InputIndex + 1];
            Red = Input[InputIndex + 2];
            
            ByteOffset = IXRes / 8;
            BitOffset = 7;
            
            uint8_t FinalPixel = ConvertTo4BPP(Red, Green, Blue);
            
            // And output the required color.
            Output[ByteOffset] |= (FinalPixel & 1) << BitOffset;
            Output[PLANE_OFFSET(1) + ByteOffset] |= ((FinalPixel & 2) >> 1) << BitOffset; 
            Output[PLANE_OFFSET(2) + ByteOffset] |= ((FinalPixel & 4) >> 2) << BitOffset;
            Output[PLANE_OFFSET(3) + ByteOffset] |= ((FinalPixel & 8) >> 3) << BitOffset;
            
            // Increase InputIndex, since j starts at '1'.
            InputIndex += 3;
            
            TBlue = TGreen = TRed = 0;
            for(uint32_t j = 1; j < (BIT.Video.XRes - 1); j++, InputIndex += 3)
            {
                // Get all the colors.
                Blue = Input[InputIndex] + TBlue;
                Green = Input[InputIndex + 1] + TGreen;
                Red = Input[InputIndex + 2] + TRed;
                if(Blue > 0xFF)
                    Blue = 0xFF;
                
                else if(Blue < 0x00)
                    Blue = 0x00;
                
                if(Green > 0xFF)
                    Green = 0xFF;
                
                else if(Green < 0x00)
                    Green = 0x00;
                
                if(Red > 0xFF)
                    Red = 0xFF;
                
                else if(Red < 0x00)
                    Red = 0x00;
        
                FinalPixel = ConvertTo4BPP(Red, Green, Blue);
                
                ByteOffset = (IXRes + j) / 8;
                BitOffset = 7 - (j % 8);
                
                // And output the required color.           
                Output[ByteOffset] |= (FinalPixel & 1) << BitOffset;
                Output[PLANE_OFFSET(1) + ByteOffset] |= ((FinalPixel & 2) >> 1) << BitOffset; 
                Output[PLANE_OFFSET(2) + ByteOffset] |= ((FinalPixel & 4) >> 2) << BitOffset;
                Output[PLANE_OFFSET(3) + ByteOffset] |= ((FinalPixel & 8) >> 3) << BitOffset;
                
                // Find the Error for each in RGB.
                Red -= (RGB4[FinalPixel] & 0xFF0000) >> 16;
                Green -= (RGB4[FinalPixel] & 0x00FF00) >> 8;
                Blue -= (RGB4[FinalPixel] & 0x0000FF);
                
                // Calculate the local versions of Y1XM1, Y1X1, Y1X.
                uint32_t LY1XM1 = InputIndex + Y1XM1;
                uint32_t LY1X1 = InputIndex + Y1X1;
                uint32_t LY1X = InputIndex + Y1X;
                
                // Dither for the particular pixel.
                // Take care of "[x - 1][y + 1]"
                // BLUE.
                TBlue = Input[LY1XM1] + ((3 * Blue) >> 4);
                if(TBlue > 0xFF)
                    TBlue = 0xFF;
    
                else if(TBlue < 0x00)
                    TBlue = 0x00;
                
                Input[LY1XM1] = (uint8_t)TBlue;
                
                // GREEN.
                TGreen = Input[LY1XM1 + 1] + ((3 * Green) >> 4);
                if(TGreen > 0xFF)
                    TGreen = 0xFF;
    
                else if(TGreen < 0x00)
                    TGreen = 0x00;
                
                Input[LY1XM1 + 1] = (uint8_t)TGreen;
    
                // RED.
                TRed = Input[LY1XM1 + 2] + ((3 * Red) >> 4);
                if(TRed > 0xFF)
                    TRed = 0xFF;
    
                else if(TRed < 0x00)
                    TRed = 0x00;
                
                Input[LY1XM1 + 2] = (uint8_t)TRed;
    
                // Take care of "[x][y + 1]"
                // BLUE.
                TBlue = Input[LY1X] + ((5 * Blue) >> 4);
                if(TBlue > 0xFF)
                    TBlue = 0xFF;
    
                else if(TBlue < 0x00)
                    TBlue = 0x00;
                
                Input[LY1X] = (uint8_t)TBlue;
    
                // GREEN.
                TGreen = Input[LY1X + 1] + ((5 * Green) >> 4);
                if(TGreen > 0xFF)
                    TGreen = 0xFF;
    
                else if(TGreen < 0x00)
                    TGreen = 0x00;
                
                Input[LY1X + 1] = (uint8_t)TGreen;
    
                // RED.
                TRed = Input[LY1X + 2] + ((5 * Red) >> 4);
                if(TRed > 0xFF)
                    TRed = 0xFF;
    
                else if(TRed < 0x00)
                    TRed = 0x00;
                
                Input[LY1X + 2] = (uint8_t)TRed;
    
                // Take care of "[x + 1][y + 1]"
                // BLUE.
                TBlue = Input[LY1X1] + (Blue >> 4);
                if(TBlue > 0xFF)
                    TBlue = 0xFF;
    
                else if(TBlue < 0x00)
                    TBlue = 0x00;
                
                Input[LY1X1] = (uint8_t)TBlue;
    
                // GREEN.
                TGreen = Input[LY1X1 + 1] + (Green >> 4);
                if(TGreen > 0xFF)
                    TGreen = 0xFF;
    
                else if(TGreen < 0x00)
                    TGreen = 0x00;
                
                Input[LY1X1 + 1] = (uint8_t)TGreen;
    
                // RED.
                TRed = Input[LY1X1 + 2] + (Red >> 4);
                if(TRed > 0xFF)
                    TRed = 0xFF;

                else if(TRed < 0x00)
                    TRed = 0x00;

                Input[LY1X1 + 2] = (uint8_t)TRed;
  
                // Take care of "[x + 1][y]", and pass it on in the loop.
                // BLUE.
                TBlue = ((7 * Blue) >> 4);
                TGreen = ((7 * Green) >> 4);
                TRed = ((7 * Red) >> 4);
            }

            // Get all the colors.
            Blue = Input[InputIndex] + TBlue;
            Green = Input[InputIndex + 1] + TGreen;
            Red = Input[InputIndex + 2] + TRed;
            
            if(Blue > 0xFF)
                Blue = 0xFF;
            
            else if(Blue < 0x00)
                Blue = 0x00;
            
            if(Green > 0xFF)
                Green = 0xFF;
            
            else if(Green < 0x00)
                Green = 0x00;
            
            if(Red > 0xFF)
                Red = 0xFF;
                       
            else if(Red < 0x00)
                Red = 0x00;
            
            // And output the required color.    
            ByteOffset = (IXRes + BIT.Video.XRes - 1) / 8;
            BitOffset = 0;
               
            Output[ByteOffset] |= (FinalPixel & 1) << BitOffset;
            Output[PLANE_OFFSET(1) + ByteOffset] |= ((FinalPixel & 2) >> 1) << BitOffset; 
            Output[PLANE_OFFSET(2) + ByteOffset] |= ((FinalPixel & 4) >> 2) << BitOffset;
            Output[PLANE_OFFSET(3) + ByteOffset] |= ((FinalPixel & 8) >> 3) << BitOffset;
                
            // And since it at ends at BIT.Video.XRes - 1, do the right pixels.
            InputIndex += 3;
        }
           
        // Do the last row.
        for(uint32_t j = 0; j < BIT.Video.XRes; j++, InputIndex += 3)
        {
            // Get all the colors.
            Blue = Input[InputIndex];
            Green = Input[InputIndex + 1];
            Red = Input[InputIndex + 2];            
                       
            uint32_t FinalPixel = ConvertTo4BPP(Red, Green, Blue);
            
            // And output the required color.
            ByteOffset = (j + IXRes) / 8;
            BitOffset = 7 - (j % 8);
            
            Output[ByteOffset] |= (FinalPixel & 1) << BitOffset;
            Output[PLANE_OFFSET(1) + ByteOffset] |= ((FinalPixel & 2) >> 1) << BitOffset; 
            Output[PLANE_OFFSET(2) + ByteOffset] |= ((FinalPixel & 4) >> 2) << BitOffset;
            Output[PLANE_OFFSET(3) + ByteOffset] |= ((FinalPixel & 8) >> 3) << BitOffset;   
        }
    }    
    
    // Else if, it is 8BPP, do the conversion.
    else if(BIT.Video.BPP == 8)
    {
        uint32_t InputIndex, OutputIndex;
        uint32_t Blue, Green, Red;
        // Temporary red, green and blue.
        uint32_t TRed, TGreen, TBlue;
        
        // NOTE: Ok, so the naming convention here, and the next function might seem weird (for variables).
        // But, I decided on it to keep it short, as well as meaningful.
        // A simple X or Y in capital, followed by another X/Y in capital (not any number or something) means
        // that it is simply the i/j index multiplied by all that it needs to be multiplied to (XRes, 3 bytes per pixel (for 24bpp)).
        // A X/Y in capital, followed by a number, means that THAT number is added to X/Y and then multiplied
        // by whatever it needs to be multiplied to (read above).
        // A X/Y in capital, followed by a M and then a number, means that the number is subtracted, and then multiplied.
        
        // So that's it. Hope I was clear :P
        uint32_t Y1X = (BIT.Video.XRes * 3), Y1XM1 = (Y1X - 3), Y1X1 = (Y1X + 3);
        
        InputIndex = OutputIndex = 0;
        
        for(uint32_t i = 0; i < (BIT.Video.YRes - 1); i++) 
        {
            // Get all the colors.
            Blue = Input[InputIndex];
            Green = Input[InputIndex + 1];
            Red = Input[InputIndex + 2];
            
            // And output the required color.
            Output[OutputIndex] = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
            
            // Increase InputIndex, since j starts at '1'.
            InputIndex += 3;
            OutputIndex++;
            
            TBlue = TGreen = TRed = 0;
            for(uint32_t j = 1; j < (BIT.Video.XRes - 1); j++, InputIndex += 3, OutputIndex++)
            {
                // Get all the colors.
                Blue = Input[InputIndex] + TBlue;
                Green = Input[InputIndex + 1] + TGreen;
                Red = Input[InputIndex + 2] + TRed;
                if(Blue > 0xFF)
                    Blue = 0xFF;
                
                if(Green > 0xFF)
                    Green = 0xFF;
                
                if(Red > 0xFF)
                    Red = 0xFF;
            
                // And output the required color.
                Output[OutputIndex] = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
                
                // Find the Error for each in RGB.
                Red &= 0x1F;
                Green &= 0x1F;
                Blue &= 0x3F;
                
                // Calculate the local versions of Y1XM1, Y1X1, Y1X.
                uint32_t LY1XM1 = InputIndex + Y1XM1;
                uint32_t LY1X1 = InputIndex + Y1X1;
                uint32_t LY1X = InputIndex + Y1X;
                
                // Dither for the particular pixel.
                // Take care of "[x - 1][y + 1]"
                // BLUE.
                TBlue = Input[LY1XM1] + ((3 * Blue) >> 4);
                if(TBlue > 0xFF)
                    TBlue = 0xFF;
    
                Input[LY1XM1] = (uint8_t)TBlue;
                
                // GREEN.
                TGreen = Input[LY1XM1 + 1] + ((3 * Green) >> 4);
                if(TGreen > 0xFF)
                    TGreen = 0xFF;
    
                Input[LY1XM1 + 1] = (uint8_t)TGreen;
    
                // RED.
                TRed = Input[LY1XM1 + 2] + ((3 * Red) >> 4);
                if(TRed > 0xFF)
                    TRed = 0xFF;
    
                Input[LY1XM1 + 2] = (uint8_t)TRed;
    
                // Take care of "[x][y + 1]"
                // BLUE.
                TBlue = Input[LY1X] + ((5 * Blue) >> 4);
                if(TBlue > 0xFF)
                    TBlue = 0xFF;
    
                Input[LY1X] = (uint8_t)TBlue;
    
                // GREEN.
                TGreen = Input[LY1X + 1] + ((5 * Green) >> 4);
                if(TGreen > 0xFF)
                    TGreen = 0xFF;
    
                Input[LY1X + 1] = (uint8_t)TGreen;
    
                // RED.
                TRed = Input[LY1X + 2] + ((5 * Red) >> 4);
                if(TRed > 0xFF)
                    TRed = 0xFF;
    
                Input[LY1X + 2] = (uint8_t)TRed;
    
                // Take care of "[x + 1][y + 1]"
                // BLUE.
                TBlue = Input[LY1X1] + (Blue >> 4);
                if(TBlue > 0xFF)
                    TBlue = 0xFF;
    
                Input[LY1X1] = (uint8_t)TBlue;
    
                // GREEN.
                TGreen = Input[LY1X1 + 1] + (Green >> 4);
                if(TGreen > 0xFF)
                    TGreen = 0xFF;
    
                Input[LY1X1 + 1] = (uint8_t)TGreen;
    
                // RED.
                TRed = Input[LY1X1 + 2] + (Red >> 4);
                if(TRed > 0xFF)
                    TRed = 0xFF;

                Input[LY1X1 + 2] = (uint8_t)TRed;
  
                // Take care of "[x + 1][y]", and pass it on in the loop.
                // BLUE.
                TBlue = ((7 * Blue) >> 4);
                TGreen = ((7 * Green) >> 4);
                TRed = ((7 * Red) >> 4);
            }

            // Get all the colors.
            Blue = Input[InputIndex] + TBlue;
            Green = Input[InputIndex + 1] + TGreen;
            Red = Input[InputIndex + 2] + TRed;
            
            if(Blue > 0xFF)
                Blue = 0xFF;
            
            if(Green > 0xFF)
                Green = 0xFF;
            
            if(Red > 0xFF)
                Red = 0xFF;
                       
            // And output the required color.
            Output[OutputIndex] = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
            
            // And since it at ends at BIT.Video.XRes - 1, do the right pixels.
            InputIndex += 3;
            OutputIndex++;
        }
           
        // Do the last row.
        for(uint32_t j = 0; j < BIT.Video.XRes; j++, InputIndex += 3, OutputIndex++)
        {
            // Get all the colors.
            Blue = Input[InputIndex];
            Green = Input[InputIndex + 1];
            Red = Input[InputIndex + 2];            
                       
            // And output the required color.
            Output[OutputIndex] = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);   
        }
    }
}

