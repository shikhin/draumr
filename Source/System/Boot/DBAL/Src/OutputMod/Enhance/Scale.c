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
void ResizeBilinear(uint8_t *Input, uint8_t *Output, uint32_t X, uint32_t Y, uint32_t NewX, uint32_t NewY) 
{
	double XNewX = ((double)(X - 1)) / ((double)(NewX - 1));
    double YNewY = ((double)(Y - 1)) / ((double)(NewY - 1));

    double  cj = 0, ci;
    uint32_t X1, Y1 = 0, X2;
    double XOff, YOff;
    
    uint32_t C1RGB[3], C2RGB[3], C3RGB[3], C4RGB[3];
    uint32_t Red, Green, Blue, Offset = 0;

    uint8_t *Line1, *Line2;
    
    X *= 3;
    for(uint32_t j = 0; j < NewY; j++, cj += YNewY, Y1 = cj)
    {
        YOff = cj - Y1;
        
        Line1 = (uint8_t*)&Input[Y1 * X];
        Line2 = (Y1 < (Y - 1) ? Line1 + X : (uint8_t*)&Input[(Y - 1) * X]);
         
        ci = 0; X1 = 0;
        for(uint32_t i = 0; i < NewX; i++, ci += XNewX, 
                                           X1 = (int)ci)
        {            
            XOff = ci  - X1;
            X1 *= 3;
            X2 = X1 < (X - 3) ? X1 + 3 : (X - 3);
            
            C1RGB[0] = Line1[X1];
            C1RGB[1] = Line1[X1 + 1];
            C1RGB[2] = Line1[X1 + 2];
            
            C2RGB[0] = Line1[X2];
            C2RGB[1] = Line1[X2 + 1];
            C2RGB[2] = Line1[X2 + 2];
            
            C3RGB[0] = Line2[X1];
            C3RGB[1] = Line2[X1 + 1];
            C3RGB[2] = Line2[X1 + 2];
                        
            C4RGB[0] = Line2[X2];
            C4RGB[1] = Line2[X2 + 1];
            C4RGB[2] = Line2[X2 + 2];
 
            // Formula.
            // Y = A(1-w)(1-h) + B(w)(1-h) + C(h)(1-w) + D(wh)
            Blue = (C1RGB[0] * ((1 - XOff) * (1 - YOff))) + 
                   (C2RGB[0] * (XOff * (1 - YOff))) +
                   (C3RGB[0] * (YOff * (1 - XOff))) +
                   (C4RGB[0] * (XOff * YOff));
   
            Green = (C1RGB[1] * ((1 - XOff) * (1 - YOff))) + 
                    (C2RGB[1] * (XOff * (1 - YOff))) +
                    (C3RGB[1] * (YOff * (1 - XOff))) +
                    (C4RGB[1] * (XOff * YOff));
            
            Red = (C1RGB[2] * ((1 - XOff) * (1 - YOff))) + 
                  (C2RGB[2] * (XOff * (1 - YOff))) +
                  (C3RGB[2] * (YOff * (1 - XOff))) +
                  (C4RGB[2] * (XOff * YOff));
			
            Output[Offset++] = Blue;
            Output[Offset++] = Green;
            Output[Offset++] = Red;
        }
    }
}
