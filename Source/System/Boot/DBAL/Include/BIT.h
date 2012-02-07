/* 
 * General BIT related definitions and structures.
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

#ifndef BIT_H     /* BIT.h */
#define BIT_H

#include <stdint.h>
#include <Output.h>
#include <BootFiles.h>

// Hardware flags.
#define A20_DISABLED    (1 << 0)

// Video flags.
#define VGA_PRESENT     (1 << 0)
#define VBE_PRESENT     (1 << 1)
#define GRAPHICAL_USED  (1 << 2)
#define TEXT_USED       (1 << 3)
#define DITHER_DISABLE  (1 << 4)
#define EDID_PRESENT    (1 << 5)

// Serial flags.
#define SERIAL_USED     (1 << 0)

// Boot device flags.
#define BD_CD           0
#define BD_FLOPPY       1
#define BD_PXE          2

// The API codes for the file access API.
#define FILE_OPEN   0
#define FILE_READ   1
#define FILE_CLOSE  2

// The BIT structure.
struct BIT
{
    // The open, read and close file functions.
    uint32_t (*FileAPI)(uint32_t APICode, ...);

    uint64_t IPS;                     // Number of instructions, executed per second.
    uint8_t  HrdwreFlags;             // The "hardware" flags.
    uint8_t  BDFlags;                 // The boot device flags.
    
    uint32_t ACPI;                    // The 32-bit address of the RSDP.
    uint32_t MPS;                     // The 32-bit address of the MPS tables.
    uint32_t SMBIOS;                  // The 32-bit address of the SMBIOS tables.

    uint32_t MMap;                    // The 32-bit address of the Memory Map.  
    
    // Define the Video related things here.
    struct
    {
        uint8_t         VideoFlags;                      // The video flags.
        VBECntrlrInfo_t *VBECntrlrInfo;                  // The 32-bit adddress of the VBE Controller Mode Info block.
        VBEModeInfo_t   *VBEModeInfo;                    // The 32-bit address of the (allocated) VBE mode info block.
        uint32_t        VBEModeInfoN;                    // The number of entries.
        
        EDIDInfo_t EDIDInfo;                             // The EDID information.
 
        uint32_t (*VideoAPI)(uint32_t APICode, ...);     // The video API function.
	    
	    uint32_t *Address;                       // The address of the video display. 
   	    uint32_t XRes;                           // X resolution.
	    uint32_t YRes;                           // Y resolution.
	    uint32_t BPP;                            // Bytes per pixel.
	    uint32_t BytesBetweenLines;              // Bytes between lines.
	
	    FILE_t   BackgroundImg;                  // Pointer to the boot image.
    } __attribute__((packed)) Video;
    
    // And the serial port related things here.
    struct
    {
        uint8_t  SerialFlags;                    // The serial port flags.
        uint32_t Port;                           // The port which we are going to use.
    } __attribute__((packed)) Serial;
    
} __attribute__((packed));

typedef struct BIT BIT_t;

// The BIT structure defined in BIT.c - where we back this up.
extern BIT_t BIT;

/*
 * Initializes the BIT structure, copying it to somewhere appropriate.
 *     uint32_t *BITPointer -> the pointer to the BIT structure, as passed to us.
 */
void BITInit(uint32_t *BITPointer);

#endif            /* BIT.h */
