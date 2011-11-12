/*Contains image converter - BMP (24bpp) to SIF.
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

// The output buffer - where we finally output the image, and then convert it out.
// It's format is 24bpp.
static uint8_t *Buffer;

struct BMPHeader
{
    uint8_t  Type[2];
    uint32_t FileSize;
    uint32_t Reserved;
    uint32_t Offset;
    
    uint32_t Size;
    uint32_t XSize, YSize;
    uint16_t Plane, BPP;
    uint32_t Compression, ImageSize;
    uint32_t XRes, YRes;
    uint16_t NColors, ImpColors;
} __attribute__((packed));

struct ImageData
{
    uint32_t XSize, YSize;
    uint32_t ImageSize;
} __attribute__((packed));

typedef struct BMPHeader BMPHeader_t;
typedef struct ImageData ImageData_t;

// Globally define ImageData.
ImageData_t ImageData;
BMPHeader_t BMPHeader;

// The function to convert BMP to the universal buffer, and fill in the details in ImageData.
// FILE *InFile                       The input file, which we would be reading.
static void BMPToBuf(FILE *InFile)
{
    uint32_t Read = 0;
    
    if(!fread(&BMPHeader, sizeof(BMPHeader_t), 1, InFile))
    {
        printf("ERROR: Unable to read the BMP header from the file.\n");
        exit(-1);
    }
    
    // Do some error checking to verify that the file we are reading is of type 'bmp'.
    // And some idiot didn't just change the extension to bpp, to auto-magically change the file type.
    if(BMPHeader.Type[0] != 'B' ||
       BMPHeader.Type[1] != 'M')
    {
        printf("ERROR: File isn't of type '.bmp', and thus the extension is incorrect.\n");
        exit(-1);
    }
    
    // Check for bpp type.
    if((BMPHeader.Plane * BMPHeader.BPP) != 24)
    {
        printf("ERROR: Image isn't of 24-bpp format, and thus can't be opened.\n\tBPP: %d\tPlane: %d\n",
               BMPHeader.Plane, BMPHeader.BPP);
        exit(-1);
    }
    
    // Check for compression type.
    if(BMPHeader.Compression != 0)
    {
        printf("ERROR: Image hasn't ""no compression"" attribute, and is compressed.\nWe can't handle stuff like that.\n");
        exit(-1);
    }
    
    // Check for palette.
    if(BMPHeader.NColors != 0)
    {
        printf("ERROR: Image has a palette appended to it, and can't be parsed.\n");
        exit(-1);
    }
    
    // Allocate a temporary buffer, where we read from file.
    uint8_t *TempBuffer = malloc(BMPHeader.ImageSize);
    // Get to the image in the file.
    fseek(InFile, BMPHeader.Offset, SEEK_SET);
    do
    {
        uint32_t Status;
        Status = fread(TempBuffer + Read, sizeof(uint8_t), (BMPHeader.ImageSize - Read), InFile);
        if(Status < 0)
        {
            printf("ERROR: Unable to read image from file.\n");
            exit(-1);
        }
        
        Read += Status;
    } while(Read < BMPHeader.ImageSize);
    Buffer = TempBuffer;
    
    // Get the data into ImageData.
    ImageData.XSize = BMPHeader.XSize;
    ImageData.YSize = BMPHeader.YSize;
    ImageData.ImageSize = BMPHeader.ImageSize;
}

// Resizes a 24bpp image.
// uint8_t  *Input
// uint8_t  *Output
// uint32_t X
// uint32_t Y
// uint32_t NewX
// uint32_t NewY
static void ResizeBilinear(uint8_t *Input, uint8_t *Output, uint32_t X, uint32_t Y, uint32_t NewX, uint32_t NewY) 
{
    uint32_t ARed, ABlue, AGreen, BRed, BBlue, BGreen, CRed, CBlue, CGreen, DRed, DGreen, DBlue;
    uint32_t x, y, i, j, Index;
    
    float XRatio = ((float)(X - 1)) / NewX;
    float YRatio = ((float)(Y - 1)) / NewY;
    float XDiff, YDiff, Blue, Red, Green;
    
    uint32_t Offset = 0;
    
    for(i = 0; i < NewY; i++) 
    {
        for(j = 0; j < NewX; j++)
        {
            x = (int)(XRatio * j);
            y = (int)(YRatio * i) ;
            XDiff = (XRatio * j) - x;
            YDiff = (YRatio * i) - y;
            
            Index = (y * X + x);                
            ARed = Input[Index * 3];
            AGreen = Input[(Index * 3) + 1];
            ABlue = Input[(Index * 3) + 2];
            
            BRed = Input[(Index + 1) * 3];
            BGreen = Input[((Index + 1) * 3) + 1];
            BBlue = Input[((Index + 1) * 3) + 2];
            
            CRed = Input[(Index + X) * 3];
            CGreen = Input[((Index + X) * 3) + 1];
            CBlue = Input[((Index + X) * 3) + 2];
            
            DRed = Input[(Index + X + 1) * 3];
            DGreen = Input[((Index + X + 1) * 3) + 1];
            DBlue = Input[((Index + X + 1) * 3) + 2];
            
            // Take care of the blue element.
            // Yb = Ab(1-w)(1-h) + Bb(w)(1-h) + Cb(h)(1-w) + Db(wh)
            Blue = ABlue * (1 - XDiff) * (1 - YDiff) + BBlue * (XDiff) * (1 - YDiff) +
                   CBlue * (YDiff) * (1 - XDiff) + DBlue *(XDiff * YDiff);
            
            Green = AGreen * (1 - XDiff) * (1 - YDiff) + BGreen * (XDiff) * (1 - YDiff) +
                    CGreen * (YDiff) * (1 - XDiff) + DGreen * (XDiff * YDiff);
            
            Red = ARed * (1 - XDiff) * (1 - YDiff) + BRed * (XDiff) * (1 - YDiff) +
                  CRed * (YDiff) * (1 - XDiff) + DRed * (XDiff * YDiff);
            
            Output[Offset++] = Red;
            Output[Offset++] = Green;
            Output[Offset++] = Blue;
        }
    }
}

// Converts the universal buffer into the SIF.
// FILE *OutFile                      The output file, where we output the final image.
static void BufToSIF(FILE *OutFile)
{
    uint8_t *TempBuffer = Buffer;
    
    Buffer = malloc(1024 * 768 * 3);
    ResizeBilinear(TempBuffer, Buffer, ImageData.XSize, ImageData.YSize, 1024, 768);
    
    BMPHeader.XSize = 1024;
    BMPHeader.YSize = 768;
    BMPHeader.FileSize = sizeof(BMPHeader_t) + 1024 * 768 * 3;
    BMPHeader.Offset = sizeof(BMPHeader_t);
    BMPHeader.Compression = 0;
    BMPHeader.ImageSize = 1024 * 768 * 3;

    fwrite(&BMPHeader, sizeof(BMPHeader_t), 1, OutFile);
    if(fwrite(Buffer, sizeof(uint8_t), 1024 * 768 * 3, OutFile) < 1024 * 768 * 3)
    {
        printf("ERROR: Unable to write the image to the output file.\n");
        exit(-1);
    }
}

// The main for the function - expects to arguments.
//     rc
//                                    0 for success, -1 for failure.
int main(int argc, char **argv)
{
    FILE *InFile, *OutFile;
    // If usage wasn't correct, gracefully exit.
    if(argc < 3)
    {
        printf("ERROR: Incorrect usage of %s.\nCorrect Usage: %s InputFile OutputFile.\n", argv[0], argv[0]);
        return -1;
    }
    
    // Open InFile and OutFile with error checking.
    InFile = fopen(argv[1], "r+b");
    OutFile = fopen(argv[2], "w+w");
    if(!InFile)
    {
        printf("ERROR: Unable to open input file %s.\n", argv[1]);
        return -1;
    }
    
    if(!OutFile)
    {
        printf("ERROR: Unable to open output file %s.\n", argv[2]);
        return -1;
    }
    
    // Find the last instance of the '.' character.
    char *Extension = strrchr(argv[1], '.');
    
    // BMP file.
    if(!strcmp(Extension, ".bmp"))
    {
        BMPToBuf(InFile);
    }
    
    // Unrecognizable file.
    else
    {
        printf("ERROR: Unable to parse the input file.\nExtension (%s) is unrecognizable.\n", Extension);
        return -1;
    }

    BufToSIF(OutFile);
                
    // Close the opened files.
    fclose(InFile);
    fclose(OutFile);
    
    // Free the buffer.
    free(Buffer);
}
