/* Contains common definitions to enhance images and stuff.
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

#ifndef ENHANCE_H                      /* Enhance.h */
#define ENHANCE_H

#include <stdint.h>

#define PLANE_OFFSET(x) (((BIT.Video.XRes + 7)/8) * BIT.Video.YRes * (x))

// Resizes a 24bpp image.
// uint8_t  *Input                    The input buffer, which we are about to resize.
// uint8_t  *Output                   The output buffer, where we will store the resized buffer.
// uint32_t X                         The previous X size of the image.
// uint32_t Y                         The previous Y size of the image.
// uint32_t NewX                      The X to be resized to.
// uint32_t NewY                      The Y to be resized to.
/* NOTE: Credit to this goes to http://tech-algorithm.com/, whose algorithm has just been slightly tweaked
 * as to work with C, and 3-channel images */
void ResizeBilinear(uint8_t *Input, uint8_t *Output, uint32_t X, uint32_t Y, uint32_t NewX, uint32_t NewY);

// Initializes dithering, filling up the lookup tables required for conversion and stuff.
void DitherInit();

// Converts a buffer to the required BPP format, INTO the DrawBoard - and dithers if required too.
// uint8_t  *Input                    The input buffer, which we are about to convert and/or dither.
// uint8_t  *Output                   The output buffer, where we will store the converted thingy.
// NOTE: The Input & Output buffer would have a size of BIT.Video.XRes * BIT.Video.YRes
void Dither(uint8_t *Input, uint8_t *Output);

#endif
