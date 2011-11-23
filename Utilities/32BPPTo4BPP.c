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
// It's format is 1bit per plane, planar format.
static uint8_t *Buffer;

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
    int32_t LowestScore = -1;
    uint8_t Color, i;
    
    for(i = 0; i < 16; i++)
    {
        int32_t RedScore = Red - ((RGB4[i] & 0xFF0000) >> 16);      
	int32_t GreenScore = Green - ((RGB4[i] & 0x00FF00) >> 8);
	int32_t BlueScore = Blue - ((RGB4[i] & 0x0000FF));
		
	Score = abs(RedScore) + abs(GreenScore) + abs(BlueScore);
        if(Score < LowestScore)
        {
            LowestScore = Score;
            Color = i;
        }
    }

    return Color;
}

#define PLANE_OFFSET(x) (((Width + 7)/8) * Height * (x))

// The main for the function - expects to arguments.
//     rc
//                                    0 for success, -1 for failure.
int main()
{
    uint8_t Pixel[3];
    FILE *File;
    char *ImageData = (char*)Data;
    uint32_t i, j;

    File = fopen("4BPPImage.h", "w");
    Buffer = calloc((Width * Height * 4) / 8, 1);
    
    for(i = 0; i < Height; i++)
    {
        for(j = 0; j < Width; j++)
	{
            HEADER_PIXEL(ImageData, Pixel);
            
	    uint8_t FinalPixel = ConvertColorTo4BPP(Pixel[0], Pixel[1], Pixel[2]);
	    
	    uint32_t ByteOffset = (j + (i * Width)) / 8;
	    uint32_t BitOffset = 7 - ((j + (i * Width)) % 8);
	    
	    Buffer[ByteOffset] |= (FinalPixel & 1) << BitOffset;
	    Buffer[PLANE_OFFSET(1) + ByteOffset] |= ((FinalPixel & 2) >> 1) << BitOffset; 
	    Buffer[PLANE_OFFSET(2) + ByteOffset] |= ((FinalPixel & 4) >> 2) << BitOffset;
	    Buffer[PLANE_OFFSET(3) + ByteOffset] |= ((FinalPixel & 8) >> 3) << BitOffset;
	}
    }
    
    fprintf(File, "#include <stdint.h>\n\n");
    
    fprintf(File, "uint8_t Image[] = \n");
    fprintf(File, "{\n");
   
    for(i = 0; i < (((Width * Height * 4) / 8) - 1); i++)
        fprintf(File, "0x%X, ", Buffer[i]);    
  
    // And the last element.
    fprintf(File, "0x%X", Buffer[(Width * Height * 4 / 8) - 2]);

    fprintf(File, "\n};");
    
    fclose(File);
    free(Buffer);
    return 0;
}
