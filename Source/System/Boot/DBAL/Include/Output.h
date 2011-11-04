/* Contains common Output definitions.
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

#ifndef VIDEO_H                       /* Video.h */
#define VIDEO_H

#include <stdint.h>

// Define the timeout for now - and fix it later to use the PIT or something.
#define MIN_TIMEOUT 100000

// VBE Controller Info - given by VBE.
struct VBECntrlrInfo
{
    /* Introduced in VBE 1.0 */
    // Should contain VESA.
    uint8_t  Signature[4];
    uint16_t Version;                            // The version of the VBE implementation.
    uint32_t OEMStringFar;                       // Far pointer to OEM String.
    uint8_t  Capabilities[4];                    // The capabilities of the graphics controller.
    uint32_t VideoModesFar;                      // Far pointer to video modes.
    uint16_t TotalMemory;                        // Number of 64KiB memory blocks.
    
    /* Introduced in VBE 2.0 */
    uint16_t OEMSoftwareRevision;                // VBE Implementation software revision.
    // Some OEM specific thingy - the name tells it all.
    uint32_t OEMVendorNameFar;
    uint32_t OEMProductNameFar;
    uint32_t OEMProductRevFar;
    
    // Some reserved area for VBE Implementation scratch, and some OEM area for the data stings.
    uint8_t  Reserved[222];
    uint8_t  OEMData[256];
} __attribute__((packed));

// VBE Mode Info array - given by VBE.
struct VBEModeInfo
{
    // And that's given by the VBE call.
    uint8_t  Data[256];
    
    // That's data used by me.
    // Well, technically, I could reduce it by *a lot*.
    // But I don't. To maintain alignment.
    uint16_t HeaderData[128];
} __attribute__((packed));

typedef struct VBECntrlrInfo VBECntrlrInfo_t;
typedef struct VBEModeInfo   VBEModeInfo_t;
// Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
// If no video card, initializes the serial port.
void OutputInit();

#endif                                /* Video.h */