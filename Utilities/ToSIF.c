/* 
 * Image file format converter - BMP (24-bpp) to SIF.
 *
 * Copyright (c) 2012, Shikhin Sethi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

// The output buffer - where we finally output the image, and then convert it out.
// It's format is 24-bpp.
static uint8_t *Buffer;

// The BMP image file format's header.
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

// Some data about the image.
struct ImageData
{
    FILE *InFile, *OutFile;

    uint32_t XSize, YSize;
    uint32_t ImageSize;
    uint32_t ScaledX, ScaledY;
} __attribute__((packed));

typedef struct BMPHeader BMPHeader_t;
typedef struct ImageData ImageData_t;

// Globally define ImageData.
static ImageData_t ImageData;
static BMPHeader_t BMPHeader;

/*
 * The function to convert BMP to the universal buffer, and fill in the details in ImageData.
 */
static void BMPToBuf()
{
    uint32_t Status, BytesRead = 0;
    
    if(!fread(&BMPHeader, sizeof(BMPHeader_t), 1, ImageData.InFile))
    {
        // Close the files - avoiding leaks.
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        perror("Unable to read the BMP header from the input file");
        exit(EXIT_FAILURE);
    }
    
    // Do some error checking to verify that the file we are reading is of type 'bmp'.
    if(BMPHeader.Type[0] != 'B' ||
       BMPHeader.Type[1] != 'M')
    {
        // Close the input file - avoiding leaks.
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        printf("ERROR: File image is not of expected bitmap file format.\n");
        exit(EXIT_FAILURE);
    }
    
    // Check for bpp type.
    if((BMPHeader.Plane * BMPHeader.BPP) != 24)
    {
        // Close the input file - avoiding leaks.
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        printf("ERROR: Input image isn't of 24-bpp format, and thus can't be opened.\n\tBPP: %d\tPlane: %d\n",
               BMPHeader.Plane, BMPHeader.BPP);
        exit(EXIT_FAILURE);
    }
    
    // Check for compression type.
    if(BMPHeader.Compression != 0)
    {
        // Close the input file - avoiding leaks.
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        printf("ERROR: Input image is compressed, and can't be handled correctly.\n");
        exit(EXIT_FAILURE);
    }
    
    // Check for palette.
    if(BMPHeader.NColors != 0)
    {
        // Close the input file - avoiding leaks.
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        printf("ERROR: Image has a palette appended to it, and can't be parsed");
        exit(EXIT_FAILURE);
    }
    
    // Allocate a temporary buffer, where we read from file.
    uint8_t *TempBuffer = malloc(BMPHeader.ImageSize);
    if(!TempBuffer)
    {
        // Close the input file - avoiding leaks.
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        perror("Unable to allocate enough space to read input image");
        exit(EXIT_FAILURE);
    }

    // Clear out temporary buffer.
    memset(TempBuffer, 0, BMPHeader.ImageSize);

    // Get to the image in the file.
    fseek(ImageData.InFile, BMPHeader.Offset, SEEK_SET);

    // Read everything in to TempBuffer.
    do
    {
        // Read the bytes left in the buffer.
        Status = fread(TempBuffer + BytesRead, sizeof(uint8_t), 
                       (BMPHeader.ImageSize - BytesRead), ImageData.InFile);

        // If some error occured during read, then..
        if(ferror(ImageData.InFile))
        {
            // Free temporary buffer, and close file.
            free(TempBuffer);
            fclose(ImageData.OutFile);
            fclose(ImageData.InFile);

            // Print error.
            perror("Unable to read image from file");
            exit(EXIT_FAILURE);
        }
        
        BytesRead += Status;
    } while(BytesRead < BMPHeader.ImageSize);
    
    Buffer = malloc(BMPHeader.ImageSize);
    if(!Buffer)
    {
        // Close the input file and free the temporary buffer - avoiding leaks.
        free(TempBuffer);
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        perror("Unable to allocate enough space to read input image");
        exit(EXIT_FAILURE);
    }
    
    // Get the data into ImageData.
    ImageData.XSize = BMPHeader.XSize;
    ImageData.YSize = BMPHeader.YSize;
    ImageData.ImageSize = BMPHeader.ImageSize;
    
    // Bytes to be copied - i.e., each line.
    uint32_t BytesToCopy = ImageData.XSize * 3;

    // Here:
    //     a) Remove the padding between lines.
    //     b) Make the image as it should be, not upside down (as is the case with BMP)
    for(uint32_t i = BMPHeader.YSize, Index = 0; i > 0; i--)
    {
        // Copy the bytes.
        memcpy(Buffer + ((i - 1) * BytesToCopy), &TempBuffer[Index], BytesToCopy);

        // And take care of the padding.
        Index += BytesToCopy + (BytesToCopy % 4);
    }
    
    // Free the temporary buffer.
    free(TempBuffer);
}

/*
 * Resizes a 24bpp image.
 *     uint8_t  *Input   -> the input buffer, which we are about to resize.
 *     uint8_t  *Output  -> the output buffer, where we will store the resized buffer.
 *     uint32_t X        -> the previous X size of the image.
 *     uint32_t Y        -> the previous Y size of the image.
 *     uint32_t NewX     -> the X to be resized to.
 *     uint32_t NewY     -> the Y to be resized to.
 */
static void ResizeBilinear(uint8_t *Input, uint8_t *Output, uint32_t X, uint32_t Y, uint32_t NewX, uint32_t NewY) 
{
    double XNewX = ((double)(X - 1)) / ((double)(NewX - 1));
    double YNewY = ((double)(Y - 1)) / ((double)(NewY - 1));

    double  cj = 0, ci;
    uint32_t X1, Y1 = 0, X2;
    double XOff, YOff;
    
    uint32_t C1RGB[3], C2RGB[3], C3RGB[3], C4RGB[3];
    uint32_t Red, Green, Blue, Offset = 0;

    uint8_t *Line1, *Line2;
    
    // Calculate the sum of the ratio of the y axic in cj.
    X *= 3;
    for(uint32_t j = 0; j < NewY; j++, cj += YNewY, Y1 = cj)
    {
        YOff = cj - Y1;
        
        // Get the two lines from where we would be taking the four reference pixels.
        Line1 = (uint8_t*)&Input[Y1 * X];
        // Get the next line - and if we have reached the end of the input buffer, then take the last line as the next line.
        Line2 = (Y1 < (Y - 1) ? Line1 + X : (uint8_t*)&Input[(Y - 1) * X]);
         
        // Calculate the sum of the ratio of the x axis in ci.
        // And the index in X1.
        ci = 0; X1 = 0;
        for(uint32_t i = 0; i < NewX; i++, ci += XNewX, 
                                           X1 = (int)ci)
        {            
            // The distance between the "actual index", and the "index".
            // (simply calculated by subtracting (int)index from index)
            XOff = ci  - X1;
            X1 *= 3;
            // If it is the end of the X axis, then take the next pixel as the last pixel.
            // Else, the next pixel.
            X2 = X1 < (X - 3) ? X1 + 3 : (X - 3);
            
            // Get the RGB values for the four pixels we'd be using to calculate the final pixel.
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
 
            // Formula used to calculate the pixel.
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
            
            // And then, store it.
            Output[Offset++] = Blue;
            Output[Offset++] = Green;
            Output[Offset++] = Red;
        }
    }
}

/*
 * Converts the universal buffer into the SIF.
 */
static void BufToSIF()
{
    // So, if we can rescale, we allocate another buffer, or we don't.
    uint8_t *TempBuffer = Buffer;
    
    if(ImageData.XSize != ImageData.ScaledX ||
       ImageData.YSize != ImageData.ScaledY)
    {
        // If we need to rescale, allocate another buffer, resize, and free the original one.
        Buffer = malloc(ImageData.ScaledX * ImageData.ScaledY * 3);
        if(!Buffer)
        {
            // Close the input file and free the temporary buffer - avoiding leaks.
            free(TempBuffer);
            fclose(ImageData.OutFile);
            fclose(ImageData.InFile);

            // Print error.
            perror("Unable to allocate enough space to resize input image");
            exit(EXIT_FAILURE);
        }

        ResizeBilinear(TempBuffer, Buffer, ImageData.XSize, ImageData.YSize, ImageData.ScaledX, ImageData.ScaledY);
        free(TempBuffer);
    
        // Get the scaled size.
        BMPHeader.XSize = ImageData.ScaledX;
        BMPHeader.YSize = ImageData.ScaledY;
    }
       
    // And other factors.
    BMPHeader.FileSize = sizeof(BMPHeader_t) + ImageData.ScaledX * ImageData.ScaledY * 3;
    BMPHeader.Offset = sizeof(BMPHeader_t);
    BMPHeader.Compression = 0;
    BMPHeader.ImageSize = ImageData.ScaledX * ImageData.ScaledY * 3;

    // Write the header, and the image.
    fwrite(&BMPHeader, sizeof(BMPHeader_t), 1, ImageData.OutFile);
    if(ferror(ImageData.OutFile))
    {
        // Free temporary buffer, and close file.
        free(Buffer);
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        perror("Unable to write the image to the output file");
        exit(EXIT_FAILURE);
    }

    fwrite(Buffer, ImageData.ScaledX * ImageData.ScaledY * 3, 1, ImageData.OutFile);
    if(ferror(ImageData.OutFile))
    {
        // Free temporary buffer, and close file.
        free(Buffer);
        fclose(ImageData.OutFile);
        fclose(ImageData.InFile);

        // Print error.
        perror("Unable to write the image to the output file");
        exit(EXIT_FAILURE);
    }
}

/*
 * The entry point to ToSIF.
 *     int argc     -> the number of arguments passed.
 *     char *argv[] -> the command line arguments string.
 *
 * Returns:
 *     int          -> EXIT_SUCCESS for success, else EXIT_FAILURE.
 */
int main(int argc, char *argv[])
{
    // If usage wasn't correct, gracefully exit.
    if(argc < 3)
    {
        printf("ERROR: Incorrect usage of %s.\nCorrect Usage: %s InputFile OutputFile Flags.\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // The default would be 1024*768.
    ImageData.ScaledX = 1024;
    ImageData.ScaledY = 768;
    
    // Open ImageData.InFile and ImageData.OutFile with error checking.
    ImageData.InFile = fopen(argv[1], "r+b");
    if(!ImageData.InFile)
    {
        // Print error.
        printf("ERROR: Unable to open input file %s.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    ImageData.OutFile = fopen(argv[2], "w+b");
    if(!ImageData.OutFile)
    {
        // Close intput file.
        fclose(ImageData.InFile);

        // Print error.
        printf("ERROR: Unable to open output file %s.\n", argv[2]);
        exit(EXIT_FAILURE);
    }
    
    // Find the last instance of the '.' character - to find the extension.
    char *Extension = strrchr(argv[1], '.');
    
    // BMP file.
    if(!strcmp(Extension, ".bmp"))
    {
        BMPToBuf(ImageData.InFile);
    }
    
    // Unrecognizable file.
    else
    {
        // Close the input and output file.
        fclose(ImageData.InFile);
        fclose(ImageData.OutFile);

        // Print error.
        printf("ERROR: Unable to parse the input file.\nExtension (%s) is unrecognizable.\n", Extension);
        exit(EXIT_FAILURE);
    }

    // Convert the buffer to SIF file format, which is actually just modified BMP.
    BufToSIF(ImageData.OutFile);
                
    // Close the opened files.
    fclose(ImageData.InFile);
    fclose(ImageData.OutFile);
    
    // Free the buffer.
    free(Buffer);
    
    printf("  [ToSIF] %s -> %s\n", argv[1], argv[2]);
}
