/* Contains common definitions for the output module.
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

#ifndef OUTPUT_MOD_H                      /* OutputMod.h */
#define OUTPUT_MOD_H

#include <stdint.h>

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

typedef struct BMPHeader BMPHeader_t;

// Initializes the output module, allocating neccessary buffers and such.
void OutputModInit();

#endif
