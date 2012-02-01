/*
 * Definitions of functions to open boot file.
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

#ifndef BOOT_FILES_H     /* Boot files.h */
#define BOOT_FILES_H

#include <stdint.h>
#include <PMM.h>

// Define the "file code" of every file, for easy usage.
#define BACKGROUND_SIF 0x02

// Bouncer has size of 64KiB.
#define BOUNCER_SIZE        (64 * 1024)
#define BOUNCER_SMALL_SIZE  (16 * 1024)

// Pages used by bouncer.
#define BOUNCER_PAGES       ((64 * 1024)/4096)
#define BOUNCER_SMALL_PAGES ((16 * 1024)/4096)

// The SIF image file format's header.
struct SIFHeader
{
    uint8_t  Type[3];
    uint32_t FileSize;
    uint32_t CRC32;
    uint32_t Offset;
    
    uint16_t Plane, BPP;
    uint32_t Compression, ImageSize;
    uint32_t XRes, YRes;
} __attribute__((packed));

// The file structure, which is returned by opening files.
typedef struct
{
	// The size of the file.
	uint32_t Size;

	// The location of the file.
    void *Location;
} FILE_t;

// Some typedef's to make stuff easier.
typedef struct SIFHeader SIFHeader_t;

/*
 * Initializes the bouncer in which we would be reading the required boot files.
 */
void BootFilesInit();

/*
 * Clears up everything initialized in the Init().
 */
void BootFilesClear();

/*
 * Gets the background image, verifying what we are getting to.
 *
 * Returns:
 *     FILE_t -> the file structure containing address and length of the file.
 */
FILE_t BootFilesBGImg();

#endif                   /* Boot files.h */