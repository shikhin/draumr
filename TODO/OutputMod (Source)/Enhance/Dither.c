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
    uint32_t Blue, Green, Red;
    // Temporary red, green and blue.
    uint32_t TRed, TGreen, TBlue;
    // The current error and next error for red, green and blue.
    uint32_t CERed, CEGreen, CEBlue, NERed, NEGreen, NEBlue;
        
    // Calculate XRes, and multiply it by 3 - we use this to find out till where to go.
    uint32_t XRes = (BIT.Video.XRes * 3);
    
    // Clear out the first and second pixels' error.
    memset(TempErrorLine, 0, sizeof(uint8_t) * 6);
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
        memset(TempErrorLine, 0, sizeof(uint8_t) * 6);
        
        // Get all the colors.
        Blue = *Input++ + CERed;
        Green = *Input++ + CEGreen;
        Red = *Input++ + CEBlue;
            
        if(Red > 0xFF)
            Red = 0xFF;
        
        if(Green > 0xFF)
            Green = 0xFF;
        
        if(Blue > 0xFF)
            Blue = 0xFF;
        
        // And output the required color.
        *Output++ = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
            
        TBlue = TGreen = TRed = 0;
        for(uint32_t j = 3; j < (XRes - 3);)
        {
            // The current errors were the next error's in the previous loop.
            CEBlue = NEBlue;
            CEGreen = NEGreen;
            CERed = NERed;
            
            // Get the next errors for blue, red and green - before we destroy it.
            NEBlue = TempErrorLine[j + 3];
            NEGreen = TempErrorLine[j + 4];
            NERed = TempErrorLine[j + 5];
            
            // Get all the colors - adding the current error for the pixel, and the error distributed from the left pixel.
            Blue = *Input++ + TBlue + CEBlue;
            Green = *Input++ + TGreen + CEGreen;
            Red = *Input++ + TRed + CERed;
            
            if(Blue > 0xFF)
                Blue = 0xFF;
               
            if(Green > 0xFF)
                Green = 0xFF;
               
            if(Red > 0xFF)
                Red = 0xFF;
          
            // And output the required color.
            *Output++ = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
               
            // Find the Error for each in RGB.
            Red &= 0x1F;
            Green &= 0x1F;
            Blue &= 0x3F;
               
            // Dither for the particular pixel.
            // Take care of "[x - 1]" in the next line.
            // Blue.
            TBlue = TempErrorLine[j - 3] + ((3 * Blue) >> 4);
            if(TBlue > 0xFF)
                TBlue = 0xFF;
    
            TempErrorLine[j - 3] = (uint8_t)TBlue;
                
            // GREEN.
            TGreen = TempErrorLine[j - 2] + ((3 * Green) >> 4);
            if(TGreen > 0xFF)
                TGreen = 0xFF;
    
            TempErrorLine[j - 2] = (uint8_t)TGreen;
   
            // Red.
            TRed = TempErrorLine[j - 1] + ((3 * Red) >> 4);
            if(TRed > 0xFF)
                TRed = 0xFF;
    
            TempErrorLine[j - 1] = (uint8_t)TRed;
    
            // Take care of "[x] for the next line"
            // Blue.
            TBlue = TempErrorLine[j] + ((5 * Blue) >> 4);
            if(TBlue > 0xFF)
                TBlue = 0xFF;
    
            TempErrorLine[j++] = (uint8_t)TBlue;
    
            // GREEN.
            TGreen = TempErrorLine[j] + ((5 * Green) >> 4);
            if(TGreen > 0xFF)
                TGreen = 0xFF;
    
            TempErrorLine[j++] = (uint8_t)TGreen;
   
            // Red.
            TRed = TempErrorLine[j] + ((5 * Red) >> 4);
            if(TRed > 0xFF)
                TRed = 0xFF;
    
            TempErrorLine[j++] = (uint8_t)TRed;
    
            // Take care of "[x + 1] for the next line"
            // Blue.
            TempErrorLine[j] = (Blue >> 4);
            
            // GREEN.
            TempErrorLine[j + 1] = (Green >> 4);
            
            // Red.
            TempErrorLine[j + 2] = (Red >> 4);
            
            // Take care of "[x + 1][y]", and pass it on in the loop.
            TBlue = ((7 * Blue) >> 4);
            TGreen = ((7 * Green) >> 4);
            TRed = ((7 * Red) >> 4);
        }

        // Get all the colors.
        Blue = *Input++ + TBlue + NEBlue;
        Green = *Input++ + TGreen + NEGreen;
        Red = *Input++ + TRed + NERed;
           
        if(Red > 0xFF)
            Red = 0xFF;
            
        if(Green > 0xFF)
            Green = 0xFF;
            
        if(Blue > 0xFF)
            Blue = 0xFF;
                       
        // And output the required color.
        *Output++ = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
    }
           
    // Do the last row.
    for(uint32_t j = 0; j < XRes;)
    {
        // Get all the colors.
        Blue = *Input++ + TempErrorLine[j++];
        Green = *Input++ + TempErrorLine[j++];
        Red = *Input++ + TempErrorLine[j++];            
                       
        if(Blue > 0xFF)
            Blue = 0xFF;
        
        if(Green > 0xFF)
            Green = 0xFF;
        
        if(Red > 0xFF)
            Red = 0xFF;
        
        // And output the required color.
        *Output++ = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);   
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
        for(uint32_t j = 0; j < BIT.Video.XRes; j++)
        {
            // Get all the colors.
            Blue = Input[InputIndex++];
            Green = Input[InputIndex++];
            Red = Input[InputIndex++];
          
            // And output the required color.
            Output[OutputIndex++] = (Red & ~0x1F) | ((Green & ~0x1F) >> 3) | ((Blue & ~0x3F) >> 6);
        }
    }
}
