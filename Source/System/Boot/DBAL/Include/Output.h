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
*  but WITHOUT ANY WARRANTY// Without even the implied warranty of
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

// Flags for ModeAttributes in VBEModeInfo.
#define HARDWARE_INIT    (1 << 0)  // If set, can be initialized. Else, can't.
#define GRAPHICAL_MODE   (1 << 4)  // If set, graphical mode. Else, text.
#define LFB_AVAILABLE    (1 << 7)  // If set, LFB is supported.

// Flags for DirectColorModeInfo.
#define COLOR_RAMP_PROGRAMMABLE (1 << 0)    // If set, color ramp is programmable.

// The factor used for scaling BPP.
#define BPP_SCALING_FACTOR (240)

// VBE Mode Info array - given by VBE.
struct VBEModeInfo
{
    // And that's given by the VBE call.
    // Mandatory information for all VBE revisions.
    uint16_t ModeAttributes;    // Mode attributes
    uint8_t  WinAAttributes;    // Window A attributes
    uint8_t  WinBAttributes;    // Window B attributes
    uint16_t WinGranularity;    // Window granularity
    uint16_t WinSize;           // Window size
    uint16_t WinASegment;       // Window A start segment
    uint16_t WinBSegment;       // Window B start segment
    uint32_t WinFuncPtr;        // Real mode pointer to window function
    uint16_t BytesPerScanLine;  // Bytes per scan line
    
    // Mandatory information for VBE 1.2 and above.
    uint16_t XResolution;       // Horizontal resolution in pixels or characters
    uint16_t YResolution;       // Vertical resolution in pixels or characters
    uint8_t  XCharSize;         // Character cell width in pixels
    uint8_t  YCharSize;         // Character cell height in pixels
    uint8_t  NumberOfPlanes;    // Number of memory planes
    uint8_t  BitsPerPixel;      // Bits per pixel
    uint8_t  NumberOfBanks;     // Number of banks
    
    uint8_t  MemoryModel;       // Memory model type
    uint8_t  BankSize;               // Bank size in KB
    uint8_t  NumberOfImagePages;     // Number of images
    uint8_t  Reserved;               // Reserved for future function 
    
    // Direct Color fields (required for direct/6 and YUV/7 memory models)
    uint8_t  RedMaskSize;       // Size of direct color red mask in bits
    uint8_t  RedFieldPosition;  // Bit position of lsb of red mask
    uint8_t  GreenMaskSize;     // Size of direct color green mask in bits
    uint8_t  GreenFieldPosition;     // Bit position of lsb of green mask
    uint8_t  BlueMaskSize;           // Size of direct color blue mask in bits
    uint8_t  BlueFieldPosition;      // Bit position of lsb of blue mask
    uint8_t  RsvdMaskSize;           // Size of direct color reserved mask in bits
    uint8_t  RsvdFieldPosition;      // Bit position of lsb of reserved mask
    uint8_t  DirectColorModeInfo;    // Direct color mode attributes
    
    // Mandatory information for VBE 2.0 and above.
    uint32_t PhysBasePtr;            // Physical address for flat memory frame buffer
    uint32_t Score;                  // Reserved - always set to 0 - but we use it for score. ;-)
    uint16_t Mode;                   // Reserved - always set to 0 - but we use it for the mode.
    
    // Mandatory information for VBE 3.0 and above.
    uint16_t LinBytesPerScanLine;    // Bytes per scan line for linear modes
    uint8_t  BnkNumberOfImagePages;  // Number of images for banked modes
    uint8_t  LinNumberOfImagePages;  // Number of images for linear modes
    uint8_t  LinRedMaskSize;         // Size of direct color red mask (linear modes)
    uint8_t  LinRedFieldPosition;    // Bit position of lsb of red mask (linear modes)
    uint8_t  LinGreenMaskSize;       // Size of direct color green mask (linear modes)
    uint8_t  LinGreenFieldPositiondb;     // Bit position of lsb of green mask (linear modes)
    uint8_t  LinBlueMaskSize;        // Size of direct color blue mask (linear modes)
    uint8_t  LinBlueFieldPosition;   // Bit position of lsb of blue mask (linear modes)
    uint8_t  LinRsvdMaskSize;        // Size of direct color reserved mask (linear modes)
    uint8_t  LinRsvdFieldPosition;   // Bit position of lsb of reserved mask (linear modes)
    uint32_t MaxPixelClock;          // Maximum pixel clock (in Hz) for graphics mode
    uint8_t  Reserved2[190];         // Remainder of ModeInfoBlock.
} __attribute__((packed));

// To verify that the video mode is in the standard RGB format - else, removes the video mode entry.
#define VERIFY_RGB_MODE(RsvdSize, RsvdPos, RedSize, RedPos,	GreenSize, GreenPos, BlueSize, BluePos) \
							  ((VBEModeInfo->RsvdMaskSize != RsvdSize)         ||  \
						       (VBEModeInfo->RsvdFieldPosition != RsvdPos)     ||  \
						       (VBEModeInfo->RedMaskSize != RedSize)           ||  \
						       (VBEModeInfo->RedFieldPosition != RedPos)       ||  \
						       (VBEModeInfo->GreenMaskSize != GreenSize)       ||  \
						       (VBEModeInfo->GreenFieldPosition != GreenPos)   ||  \
						       (VBEModeInfo->BlueMaskSize != BlueSize)         ||  \
						       (VBEModeInfo->BlueFieldPosition != BluePos))        						

typedef struct VBECntrlrInfo VBECntrlrInfo_t;
typedef struct VBEModeInfo   VBEModeInfo_t;

// Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
// If no video card, initializes the serial port.
void OutputInit();

#endif                                /* Video.h */
