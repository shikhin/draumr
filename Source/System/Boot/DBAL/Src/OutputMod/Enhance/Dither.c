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

/* 8BPP STUFF */

// Converts a buffer to the required BPP format, INTO the DrawBoard - and dithers if required too.
// uint8_t  *Input                    The input buffer, which we are about to convert and/or dither.
// uint8_t  *Output                   The output buffer, where we will store the converted thingy.
// NOTE: The Input & Output buffer would have a size of BIT.Video.XRes * BIT.Video.YRes
void Dither8BPP(uint8_t *Input, uint8_t *Output)
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

// Converts a buffer to the required BPP format, INTO the DrawBoard.
// uint8_t  *Input                    The input buffer, which we are about to convert.
// uint8_t  *Output                   The output buffer, where we will store the converted thingy.
// NOTE: The Input & Output buffer would have a size of BIT.Video.XRes * BIT.Video.YRes
void Convert8BPP(uint8_t *Input, uint8_t *Output)
{
    uint32_t InputIndex, OutputIndex;
    uint32_t Blue, Green, Red;
        
    InputIndex = OutputIndex = 0;
        
    for(uint32_t i = 0; i < BIT.Video.YRes; i++) 
    {
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

