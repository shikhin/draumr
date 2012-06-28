/*
 * Definitions of functions to open, close and read boot files.
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
 *     * Neither the name of Draumr nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <BootFiles.h>
#include <String.h>
#include <PMM.h>
#include <Abort.h>
#include <BIT.h>
#include <Macros.h>
#include <Log.h>

// The bouncer where we'd load files.
static void *Bouncer = (void*)NULL;

// The size of the bouncer.
static uint32_t BouncerSize = BOUNCER_SIZE;

// For CRC checking of boot files.
uint32_t Table[256] =
{   0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D };

/*
 * Creates CRC for file data.
 *     uint32_t Seed        -> the seed from which to start calculating the CRC value.
 *     uint32_t Size        -> the size of the input buffer.
 *     uint8_t *InputBuffer -> the input buffer for which to calculate the CRC value.
 *
 * Returns:
 *     uint32_t             -> the CRC value.
 */
static uint32_t CRC(uint32_t Seed, uint32_t Size, uint8_t *InputBuffer)
{
    while(Size--)
    {
        Seed = Table[ ((uint8_t)Seed ^ * (InputBuffer++))] ^ (Seed >> 8);
    }

    return Seed;
}

/*
 * Initializes the bouncer in which we would be reading the required boot files.
 */
void BootFilesInit()
{
    // Try to allocate the big size - recommended.
    Bouncer = (void*)PMMAllocContigFrames(BASE_BITMAP, BOUNCER_PAGES);

    // If we were unable to allocate a page, then, try the small size.
    if(!Bouncer)
    {
        Bouncer = (void*)PMMAllocContigFrames(BASE_BITMAP, BOUNCER_SMALL_PAGES);

        // If we were unable to allocate the small size, then abort boot.
        if(!Bouncer)
        {
            AbortBoot("Unable to allocate enough pages for bouncer.\n");
        }

        BouncerSize = BOUNCER_SMALL_SIZE;
    }
}

/*
 * Clears up everything initialized in the Init().
 */
void BootFilesClear()
{
    // Free the bouncer, of size, BouncerSize.
    PMMFreeContigFrames((uint32_t)Bouncer, BouncerSize);
}

/*
 * Gets the background image, verifying what we are getting to.
 *
 * Returns:
 *     FILE_t -> the file structure containing address and length of the file.
 */
FILE_t BootFilesBGImg()
{
    // The file structure in which we'd store data about the Background image file.
    FILE_t File;

    // The SIFFile header.
    SIFHeader_t *SIFHeader;

    // Open the file, with code, BACKGROUND_SIF.
    File.Size = BIT.FileAPI(FILE_OPEN, (uint32_t)BACKGROUND_SIF);

    // If we were unable to open it, return blank file structure.
    if(!File.Size)
    {
        // Close the file, anyway.
        BIT.FileAPI(FILE_CLOSE);

        // And return with the File structure.
        return File;
    }

    // Read 2048 bytes at the bouncer.
    BIT.FileAPI(FILE_READ, (uint32_t*)Bouncer, 2048);

    SIFHeader = (SIFHeader_t*)Bouncer;
    // Check the file signature.
    if((SIFHeader->Type[0] != 'S') || (SIFHeader->Type[1] != 'I')
        || (SIFHeader->Type[2] != 'F'))
    {
        // If it didn't match, make size 0.
        File.Size = 0;

        // Close the file.
        BIT.FileAPI(FILE_CLOSE);

        // And return the file structure.
        return File;
    }

    if(((File.Size + 0xFFF) / 0x1000)
         != ((SIFHeader->FileSize + 0xFFF) / 0x1000))
    {
        // File size returned by FS, and in the image, didn't match.
        File.Size = 0;

        // Close the file.
        BIT.FileAPI(FILE_CLOSE);

        return File;
    }

    // Allocate enough space to hold the file.
    File.Location = (void*)PMMAllocContigFrames(POOL_BITMAP,
            (File.Size + 0xFFF) / 0x1000);

    // So we can't allocate space for the image.
    if(!File.Location)
    {
        // Make size 0.
        File.Size = 0;

        // Close the file.
        BIT.FileAPI(FILE_CLOSE);

        // Return with the file structure.
        return File;
    }

    // Reduce the 2048 bytes we left and read the rest of the file.
    uint32_t Size = File.Size - 2048;

    uint8_t *OutputBuffer = (uint8_t*)File.Location;

    // Copy the 2048 bytes we read.
    memcpy(OutputBuffer, Bouncer, 2048);

    // Update the address of the SIF Header.
    SIFHeader = (SIFHeader_t*)OutputBuffer;

    OutputBuffer += 2048;

    // Keep reading "BouncerSize" bytes in the bouncer, and copy them to the output buffer.
    while(Size >= BouncerSize)
    {
        BIT.FileAPI(FILE_READ, (uint32_t*)Bouncer, BouncerSize);
        memcpy(OutputBuffer, Bouncer, BouncerSize);

        Size -= BouncerSize;
        OutputBuffer += BouncerSize;
    }

    // If they are any left over bytes, read them.
    if(Size)
    {
        BIT.FileAPI(FILE_READ, (uint32_t*)Bouncer, Size);
        memcpy(OutputBuffer, Bouncer, Size);
    }

    // If CRC values are not equal. 
    if(SIFHeader->CRC32
       != ~CRC(0xFFFFFFFF, SIFHeader->ImageSize,
               (uint8_t*)File.Location + SIFHeader->Offset))
    {
        // Free the space we allocated for the Image.
        PMMFreeContigFrames((uint32_t)File.Location,
                (File.Size + 0xFFF) / 0x1000);

        // Make file size 0.
        File.Size = 0;

        // Close the file.
        BIT.FileAPI(FILE_CLOSE);

        return File;
    }

    BIT.FileAPI(FILE_CLOSE);
    return File;
}

/*
 * Gets the Kernel Loader file, verifying what we are getting to.
 *
 * Returns:
 *     FILE_t -> the file structure containing address and length of the file.
 *     Boot   -> aborts boot if unable to load the KL.
 */
FILE_t BootFilesKL()
{
    // The file structure in which we'd store data about the KL file.
    FILE_t File;

    // Open the file, with code, KL.
    File.Size = BIT.FileAPI(FILE_OPEN, KL);

    // If we were unable to open it, return blank file structure.
    if(!File.Size)
    {
        AbortBoot("Unable to open the KL file.\n");
    }

    // Read 2048 bytes at KL_LOCATION.
    BIT.FileAPI(FILE_READ, (uint32_t*)KL_LOCATION, 2048);

    BootFileHeader_t *Header = (BootFileHeader_t*)KL_LOCATION;

    // Check the file signature.
    if((Header->Signature[0] != ' ') || (Header->Signature[1] != ' ')
       || (Header->Signature[2] != 'K') || (Header->Signature[3] != 'L'))
    {
        AbortBoot("Corrupt KL header.\n");
    }

    if(((File.Size + 0xFFF) / 0x1000)
         != (( (Header->FileEnd - Header->FileStart) + 0xFFF) / 0x1000))
    {
        AbortBoot("Corrupt KL header.\n");
    }

    // Allocate enough space to hold the file.
    File.Location = (void*)KL_LOCATION;

    // Reduce the 2048 bytes we left and read the rest of the file.
    uint32_t FileSize = File.Size;
    if(FileSize < 2048)
    {
        FileSize = 0;
    }

    else
    {
        FileSize -= 2048;
    }

    uint8_t *OutputBuffer = (uint8_t*)File.Location;
    OutputBuffer += 2048;

    // Keep reading "BouncerSize" bytes.
    while(FileSize >= BouncerSize)
    {
        BIT.FileAPI(FILE_READ, OutputBuffer, BouncerSize);

        FileSize -= BouncerSize;
        OutputBuffer += BouncerSize;
    }

    // If they are any left over bytes, read them.
    if(FileSize)
    {
        BIT.FileAPI(FILE_READ, OutputBuffer, FileSize);
    }

    // If CRC values are not equal. 
    if(Header->CRC32
        != ~CRC(0xFFFFFFFF,
                (Header->FileEnd - Header->FileStart)
                                 - sizeof(BootFileHeader_t),
                (uint8_t*)File.Location + sizeof(BootFileHeader_t)))
    {
        DebugPrintText("%x\n", Header->FileEnd);
        AbortBoot("Incorrect CRC32 value of the KL file.\n");
    }

    BIT.FileAPI(FILE_CLOSE);
    return File;
}

/*
 * Gets the x86 kernel, verifying what we are getting too.
 *     uint32-t Arch -> the arch of the kernel to load.
 *
 *                      0x00 -> x86
 *                      0x01 -> AMD64
 *
 * Returns:
 *     FILE_t        -> the file structure containing address and length of the file.
 *     Boot          -> aborts boot if we are unable to load the kernel.
 */
FILE_t BootFilesKernel(uint32_t Arch)
{
    // The file structure in which we'd store data about the kernel (file).
    FILE_t File;

    uint32_t FileCode;
    const uint8_t *String;
    switch (Arch)
    {
        case ARCH_X86:
            FileCode = KERNEL_X86;
            // Initialize string for error messages.
            String = (uint8_t*)"x86";

            break;

        case ARCH_AMD64:
            FileCode = KERNEL_AMD64;
            // Initialize string for error messages.
            String = (uint8_t*)"AMD64";

            break;

        default:
            // Switch to text mode.
            BIT.Video.VideoAPI(VIDEO_VGA_SWITCH_MODE, MODE_80_25_TEXT);

            AbortBoot(
                    "Unrecognizable architecture passed to file API for kernel.");
            break;
    }

    // Open the file, with code, FileCode.
    File.Size = BIT.FileAPI(FILE_OPEN, FileCode);

    // If we were unable to open it, abort.
    if(!File.Size)
    {
        // Switch to text mode.
        BIT.Video.VideoAPI(VIDEO_VGA_SWITCH_MODE, MODE_80_25_TEXT);

        DebugPrintText("%s kernel!\n", String);
        AbortBoot("Unable to open the kernel file.\n");
    }

    // Read 2048 bytes at the bouncer.
    BIT.FileAPI(FILE_READ, (uint32_t*)Bouncer, 2048);

    KernelHeader_t *KernelHeader = (KernelHeader_t*)Bouncer;

    // Check the file signature.
    if((KernelHeader->Signature[0] != 'K')
        || (KernelHeader->Signature[1] != 'E'))
    {
        // Switch to text mode.
        BIT.Video.VideoAPI(VIDEO_VGA_SWITCH_MODE, MODE_80_25_TEXT);

        DebugPrintText("%s kernel!\n", String);
        AbortBoot("Corrupt Kernel header.");
    }

    if(((Arch == ARCH_X86)
      && (KernelHeader->Signature[2] != '3'
      || KernelHeader->Signature[3] != '2'))

     ||

        ((Arch == ARCH_AMD64)
      && (KernelHeader->Signature[2] != '6'
      || KernelHeader->Signature[3] != '4')))
    {
        // Switch to text mode.
        BIT.Video.VideoAPI(VIDEO_VGA_SWITCH_MODE, MODE_80_25_TEXT);

        DebugPrintText("%s kernel!\n", String);
        AbortBoot("Kernel header's architecture does not match file name.");
    }

    if(((File.Size + 0xFFF) / 0x1000)
        != (((KernelHeader->FileEnd - KernelHeader->FileStart) + 0xFFF)
                    / 0x1000))
    {
        // Switch to text mode.
        BIT.Video.VideoAPI(VIDEO_VGA_SWITCH_MODE, MODE_80_25_TEXT);

        DebugPrintText("%s kernel!\n", String);
        AbortBoot("Corrupt Kernel header.");
    }

    // Allocate enough space to hold the file.
    File.Location = (void*)PMMAllocContigFrames(POOL_BITMAP,
            (File.Size + 0xFFF) / 0x1000);

    // So we can't allocate space for the image.
    if(!File.Location)
    {
        // Switch to text mode.
        BIT.Video.VideoAPI(VIDEO_VGA_SWITCH_MODE, MODE_80_25_TEXT);

        DebugPrintText("%s kernel!\n", String);
        AbortBoot("Cannot allocate enough space for Kernel file.");
    }

    // Reduce the 2048 bytes we left and read the rest of the file.
    uint32_t FileSize = File.Size;
    if(FileSize < 2048)
    {
        FileSize = 0;
    }

    else
    {
        FileSize -= 2048;
    }

    uint8_t *OutputBuffer = (uint8_t*)File.Location;

    // Copy the 2048 bytes we read.
    memcpy(OutputBuffer, Bouncer, 2048);

    // Update the address of the SIF Header.
    KernelHeader = (KernelHeader_t*)OutputBuffer;

    OutputBuffer += 2048;

    // Keep reading "BouncerSize" bytes in the bouncer, and copy them to the output buffer.
    while(FileSize >= BouncerSize)
    {
        BIT.FileAPI(FILE_READ, (uint32_t*)Bouncer, BouncerSize);
        memcpy(OutputBuffer, Bouncer, BouncerSize);

        FileSize -= BouncerSize;
        OutputBuffer += BouncerSize;
    }

    // If they are any left over bytes, read them.
    if(FileSize)
    {
        BIT.FileAPI(FILE_READ, (uint32_t*)Bouncer, FileSize);
        memcpy(OutputBuffer, Bouncer, FileSize);
    }

    // If CRC values are not equal.
    if(KernelHeader->CRC32
       != ~CRC(0xFFFFFFFF,
               ((KernelHeader->FileEnd - KernelHeader->FileStart)
                                       - sizeof(KernelHeader_t)),
               (uint8_t*)File.Location + sizeof(KernelHeader_t)))
    {
        // Switch to text mode.
        BIT.Video.VideoAPI(VIDEO_VGA_SWITCH_MODE, MODE_80_25_TEXT);

        DebugPrintText("%s kernel!\n", String);
        AbortBoot("Incorrect CRC32 value of the Kernel file.");
    }

    BIT.FileAPI(FILE_CLOSE);
    return File;
}
