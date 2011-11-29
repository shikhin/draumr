/* Contains common definitions to scale images and stuff.
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
#include <OutputMod/Enhance.h>

// Resizes a 24bpp image.
// uint8_t  *Input                    The input buffer, which we are about to resize.
// uint8_t  *Output                   The output buffer, where we will store the resized buffer.
// uint32_t X                         The previous X size of the image.
// uint32_t Y                         The previous Y size of the image.
// uint32_t NewX                      The X to be resized to.
// uint32_t NewY                      The Y to be resized to.
/* NOTE: Credit to this goes to http://tech-algorithm.com/, whose algorithm has just been slightly tweaked
 * as to work with C, and 3-channel images */
void ResizeBilinear(uint8_t *Input, uint8_t *Output, uint32_t X, uint32_t Y, uint32_t NewX, uint32_t NewY) 
{
    uint32_t ARed, ABlue, AGreen, BRed, BBlue, BGreen, CRed, CBlue, CGreen, DRed, DGreen, DBlue;
    uint32_t i, j, OldIndex, Index;
    
    float XRatio = ((float)(X - 1)) / NewX;
    float YRatio = ((float)(Y - 1)) / NewY;
    float XDiff, YDiff, Blue, Red, Green;
    
    X *= 3;
    uint32_t Offset = 0;
    
    for(i = 0; i < NewY; i++) 
    {
        YDiff = (YRatio * i);
        OldIndex = (int)YDiff * X;   
        YDiff -= (int)(YDiff);
        
        for(j = 0; j < NewX; j++)
        {
            XDiff = (XRatio * j);
            Index = OldIndex + ((int)XDiff * 3);      
            XDiff -= (int)XDiff;
            
            ARed = Input[Index];
            AGreen = Input[Index + 1];
            ABlue = Input[Index + 2];
            
            BRed = Input[Index + 3];
            BGreen = Input[Index + 4];
            BBlue = Input[Index + 5];
            
            CRed = Input[Index + X];
            CGreen = Input[Index + X + 1];
            CBlue = Input[Index + X + 2];
            
            DRed = Input[Index + X + 3];
            DGreen = Input[Index + X + 4];
            DBlue = Input[Index + X + 5];
            
            // Take care of the blue element.
            // Yb = Ab(1-w)(1-h) + Bb(w)(1-h) + Cb(h)(1-w) + Db(wh)
            Blue = ABlue * (1 - XDiff) * (1 - YDiff) + BBlue * (XDiff) * (1 - YDiff) +
            CBlue * (YDiff) * (1 - XDiff) + DBlue * (XDiff * YDiff);
            
            // And green.
            Green = AGreen * (1 - XDiff) * (1 - YDiff) + BGreen * (XDiff) * (1 - YDiff) +
            CGreen * (YDiff) * (1 - XDiff) + DGreen * (XDiff * YDiff);
            
            // And red.
            Red = ARed * (1 - XDiff) * (1 - YDiff) + BRed * (XDiff) * (1 - YDiff) +
            CRed * (YDiff) * (1 - XDiff) + DRed * (XDiff * YDiff);
            
            Output[Offset++] = Red;
            Output[Offset++] = Green;
            Output[Offset++] = Blue;
        }
    }
}