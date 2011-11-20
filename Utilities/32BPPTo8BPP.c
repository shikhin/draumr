/*Contains image converter - and expects Image.h to be present in include folder.
* And contain the image.
* In 24BPP format - 32 is just for making it sound nice.
*
* Copyright (c) 2011 Shikhin Sethi
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

// The output buffer - where we finally output the image, and the print it out.
// It's format is 8bit per pixel.
static uint8_t *Buffer;

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
    uint16_t ERed = (Red + 0xF) & ~0x1F;
    uint16_t EGreen = (Green + 0xF) & ~0x1F;
    uint16_t EBlue = (Blue + 0xF) & ~0x3F;
    
    // If it overflowed, then make it the maximum possible.
    if(ERed > 0xE0)
        ERed = 0xE0;
    
    if(EGreen > 0xE0)
        EGreen = 0xE0;
    
    if(EBlue > 0xC0)
        EBlue = 0xC0;
    
    // Find the top most 3 bits.
    Color |= (uint8_t)ERed;
    Color |= (uint8_t)(EGreen >> 3);
    Color |= (uint8_t)(EBlue >> 6);
    
    return Color;
}

// The main for the function - expects to arguments.
//     rc
//                                    0 for success, -1 for failure.
int main()
{
    uint8_t Pixel[3];
    FILE *File;
    char *ImageData = (char*)Data;
    uint32_t i;

    File = fopen("8BPPImage.h", "w");
    Buffer = calloc(Width * Height, 1);
    
    // For every byte, convert it.
    for(i = 0; i < (Height * Width); i++)
    {
        HEADER_PIXEL(ImageData, Pixel);
        Buffer[i] = ConvertColorTo8BPP(Pixel[0], Pixel[1], Pixel[2]);
    }
        
    fprintf(File, "#include <stdint.h>\n\n");
    
    fprintf(File, "uint8_t Image[] = \n");
    fprintf(File, "{\n");
   
    for(i = 0; i < ((Width * Height) - 1); i++)
        fprintf(File, "0x%X, ", Buffer[i]);    
  
    // And the last element.
    fprintf(File, "0x%X", Buffer[(Width * Height) - 2]);

    fprintf(File, "\n}; __attribute__((packed))");
    
    fclose(File);
    free(Buffer);
    return 0;
}
