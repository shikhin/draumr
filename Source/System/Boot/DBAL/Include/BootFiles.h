/*  Definitions of functions to open boot file.
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

#ifndef BOOT_FILES_H     /* Boot files.h */
#define BOOT_FILES_H

#include <stdint.h>
#include <PMM.h>

// Define the "file code" of every file, for easy usage.
#define BACKGROUND_SIF 0x02

// Bouncer has size of 64KiB.
#define BOUNCER_SIZE   (64 * 1024)
#define BOUNCER_SMALL_SIZE (16 * 1024)

// Pages used by bouncer.
#define BOUNCER_PAGES  ((64 * 1024)/4096)
#define BOUNCER_SMALL_PAGES ((16 * 1024)/4096)

// The file structure, which is returned by opening files.
struct FILE
{
    void *Location;
    uint32_t Size;
} __attribute__((packed));

typedef struct FILE FILE_t;

extern void *Bouncer;

// Initializes the bouncer in which we would be reading the required boot files.
void InitBootFiles();

// Clears up everything initialized in the Init().
void ClearBootFiles();

// Gets the background image, verifying what we are getting to.
//     rc
//                                    FILE_t - the file structure containing address and length of the file.
FILE_t BootFilesBGImg();

#endif                   /* Boot files.h */