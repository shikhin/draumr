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

// The temporary buffer, where we keep the error for the next line.
extern uint8_t  *TempErrorLine;

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
    // The current error and next error for red, green and blue.
    uint32_t CERed, CEGreen, CEBlue, NERed, NEGreen, NEBlue;
    
    InputIndex = OutputIndex = 0;
        
    // Clear out the first and second pixels' error.
    TempErrorLine[0] = TempErrorLine[1] = TempErrorLine[2] = 0;
    TempErrorLine[3] = TempErrorLine[4] = TempErrorLine[5] = 0;
    for(uint32_t i = 0; i < (BIT.Video.YRes - 1); i++) 
    {
        // Get the current error for blue, green and red.
        CEBlue = TempErrorLine[0];
        CEGreen = TempErrorLine[1];
        CERed = TempErrorLine[2];
        
        // And the next error for the same.
        NEBlue = TempErrorLine[3];
        NEGreen = TempErrorLine[4];
        NERed = TempErrorLine[5];
        
        // Clear out the first and second pixels' error.
        TempErrorLine[0] = TempErrorLine[1] = TempErrorLine[2] = 0;
        TempErrorLine[3] = TempErrorLine[4] = TempErrorLine[5] = 0;
        
        // Get all the colors.
        Blue = Input[InputIndex] + CEBlue;
        Green = Input[InputIndex + 1] + CEGreen;
        Red = Input[InputIndex + 2] + CERed;
            
        if(Blue > 0xFF)
            Blue = 0xFF;
        
        if(Green > 0xFF)
            Green = 0xFF;
        
        if(Red > 0xFF)
            Red = 0xFF;
        
        // And output the required color.
        Output[OutputIndex] = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
       
        // Increase InputIndex, since j starts at '1'.
        InputIndex += 3;
        OutputIndex++;
            
        TBlue = TGreen = TRed = 0;
        for(uint32_t j = 1; j < (BIT.Video.XRes - 1); j++, InputIndex += 3, OutputIndex++)
        {
            // The current errors were the next error's in the previous loop.
            CEBlue = NEBlue;
            CEGreen = NEGreen;
            CERed = NERed;
            
            // Get the next errors for blue, red and green - before we destroy it.
            NEBlue = TempErrorLine[(j + 1) * 3];
            NEGreen = TempErrorLine[((j + 1) * 3) + 1];
            NERed = TempErrorLine[((j + 1) * 3) + 2];
            
            // Get all the colors - adding the current error for the pixel, and the error distributed from the left pixel.
            Blue = Input[InputIndex] + TBlue + CEBlue;
            Green = Input[InputIndex + 1] + TGreen + CEGreen;
            Red = Input[InputIndex + 2] + TRed + CERed;
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
               
            // Dither for the particular pixel.
            // Take care of "[x - 1]" in the next line.
            // BLUE.
            TBlue = TempErrorLine[((j - 1) * 3)] + ((3 * Blue) >> 4);
            if(TBlue > 0xFF)
                TBlue = 0xFF;
    
            TempErrorLine[((j - 1) * 3)] = (uint8_t)TBlue;
                
            // GREEN.
            TGreen = TempErrorLine[((j - 1) * 3) + 1] + ((3 * Green) >> 4);
            if(TGreen > 0xFF)
                TGreen = 0xFF;
    
            TempErrorLine[((j - 1) * 3) + 1] = (uint8_t)TGreen;
    
            // RED.
            TRed = TempErrorLine[((j - 1) * 3) + 2] + ((3 * Red) >> 4);
            if(TRed > 0xFF)
                TRed = 0xFF;
    
            TempErrorLine[((j - 1) * 3) + 2] = (uint8_t)TRed;
   
            // Take care of "[x] for the next line"
            // BLUE.
            TBlue = TempErrorLine[(j * 3)] + ((5 * Blue) >> 4);
            if(TBlue > 0xFF)
                TBlue = 0xFF;
    
            TempErrorLine[(j * 3)] = (uint8_t)TBlue;
    
            // GREEN.
            TGreen = TempErrorLine[(j * 3) + 1] + ((5 * Green) >> 4);
            if(TGreen > 0xFF)
                TGreen = 0xFF;
    
            TempErrorLine[(j * 3) + 1] = (uint8_t)TGreen;
    
            // RED.
            TRed = TempErrorLine[(j * 3) + 2] + ((5 * Red) >> 4);
            if(TRed > 0xFF)
                TRed = 0xFF;
    
            TempErrorLine[(j * 3) + 2] = (uint8_t)TRed;
    
            // Take care of "[x + 1] for the next line"
            // BLUE.
            TempErrorLine[((j + 1) * 3)] = 0;
            TempErrorLine[((j + 1) * 3) + 1] = 0;
            TempErrorLine[((j + 1) * 3) + 2] = 0;
            
            TBlue = TempErrorLine[((j + 1) * 3)] + (Blue >> 4);
            if(TBlue > 0xFF)
                TBlue = 0xFF;
    
            TempErrorLine[((j + 1) * 3)] = (uint8_t)TBlue;
    
            // GREEN.
            TGreen = TempErrorLine[((j + 1) * 3) + 1] + (Green >> 4);
            if(TGreen > 0xFF)
                TGreen = 0xFF;
    
            TempErrorLine[((j + 1) * 3) + 1] = (uint8_t)TGreen;
    
            // RED.
            TRed = TempErrorLine[((j + 1) * 3) + 2] + (Red >> 4);
            if(TRed > 0xFF)
                TRed = 0xFF;

            TempErrorLine[((j + 1) * 3) + 2] = (uint8_t)TRed;
  
            // Take care of "[x + 1][y]", and pass it on in the loop.
            // BLUE.
            TBlue = ((7 * Blue) >> 4);
            TGreen = ((7 * Green) >> 4);
            TRed = ((7 * Red) >> 4);
        }

        // Get all the colors.
        Blue = Input[InputIndex] + TBlue + NEBlue;
        Green = Input[InputIndex + 1] + TGreen + NEGreen;
        Red = Input[InputIndex + 2] + TRed + NERed;
           
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
        Blue = Input[InputIndex] + TempErrorLine[j * 3];
        Green = Input[InputIndex + 1] + TempErrorLine[(j * 3) + 1];
        Red = Input[InputIndex + 2] + TempErrorLine[(j * 3) + 2];            
                       
        if(Blue > 0xFF)
            Blue = 0xFF;
        
        if(Green > 0xFF)
            Green = 0xFF;
        
        if(Red > 0xFF)
            Red = 0xFF;
        
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

