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
 *     * Neither the name of Draumr nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * - INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION - HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _BIT_H
#define _BIT_H

#include <Standard.h>

// Hardware flags.
#define A20_DISABLED    (1 << 0)

// Video flags.
#define VGA_PRESENT     (1 << 0)
#define VBE_PRESENT     (1 << 1)
#define EDID_PRESENT    (1 << 2)

// Boot device flags.
#define BD_CD           0
#define BD_FLOPPY       1
#define BD_PXE          2

// The API codes for the video handling API.
#define VIDEO_VGA_SWITCH_MODE   0
#define VIDEO_VGA_PALETTE       1

#define VIDEO_VBE_SWITCH_MODE   10
#define VIDEO_VBE_PALETTE       11
#define VIDEO_VBE_GET_MODES     12

#define VIDEO_OUTPUT_REVERT     20

// The API codes for the file access API.
#define FILE_OPEN      0
#define FILE_READ      1
#define FILE_CLOSE     2

#define FILE_KERNEL    10

// Architecture.
#define ARCH_X86       0x00
#define ARCH_AMD64     0x01

// The file structure, which is returned by opening files.
typedef struct
{
    // The size of the file.
    uint32_t Size;

    // The location of the file.
    void     *Location;
} FILE_t;

// EDID Info - taken from the BIOS again.
struct EDIDInfo
{
    // The header - to easily recognize it from other bytes (or so the EDID specs say).
    uint8_t  Header[8];

    // The vendor/product identification fields.
    // A 3-character manufacturer ID - to identify the manufacturer.
    uint8_t  ManufacturerID[2];
    // Vendor asigned product code.
    uint8_t  ProductCode[2];
    // Serial number of the monitor.
    uint32_t SerialNumber;
    // Week and year of manufacture.
    uint8_t  Week;
    uint8_t  Year;

    // EDID structure version.
    // For 1.3 - would be Version 1, Revision 3.
    uint8_t  Version;
    uint8_t  Revision;

    // Basic display parameters and features.
    // Various flags which I won't be covering here.
    uint8_t  InputDefinition;
    // Defined in cm.
    uint8_t  HorizontalHSize;
    uint8_t  VerticalSize;
    // Gamma.
    // Present as "(Gamma * 100) - 100"
    uint8_t  Gamma;
    // Some other flags.
    uint8_t  Features;

    // Filter chromacity.
    uint8_t  Red;
    uint8_t  Blue;
    uint8_t  Red_x;
    uint8_t  Red_y;
    uint8_t  Green_x;
    uint8_t  Green_y;
    uint8_t  Blue_x;
    uint8_t  Blue_y;
    uint8_t  White_x;
    uint8_t  White_y;

    // Established timings - used to indicate support for some common timings.
    uint8_t  EstablishedTimings[3];

    // Standard timings - used to identify future standard timings not defined in 
    // established timings.
    uint8_t  StandardTimings[8][2];

    // Detailed timings - used to identify timings not present in the above.
    uint8_t  DetailedTimings[4][18];

    // Number of optional 128-byte EDID blocks to follow.
    uint8_t  ExtensionFlag;
    // The checksum - makes the entire table 0.
    uint8_t  Checksum;
} _PACKED;

typedef struct EDIDInfo EDIDInfo_t;

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
    uint8_t  Reserved0;              // Reserved for future function 
    
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
    
    // While the reserved block is actually 190 bytes long, we use some of it for storing data.
    uint8_t  Reserved1[182];         // Remainder of ModeInfoBlock.

    float    MonitorPreference;
    uint32_t BytesBetweenLines;      // Bytes between lines.
} _PACKED;

typedef struct VBEModeInfo VBEModeInfo_t;

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
} _PACKED;

typedef struct VBECntrlrInfo VBECntrlrInfo_t;

// The BIT structure.
struct BIT
{
    // The open, read and close file functions.
    uint32_t (*FileAPI)(uint32_t APICode, ...);

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
        void     (*OutputRevert)(void);                  // The function to revert back from the current level.
        
        VBEModeInfo_t ModeInfo;                          // The mode we switched to's information.
    
        FILE_t   BackgroundImg;                          // Pointer to the boot image.
    } _PACKED Video;    
} _PACKED;

typedef struct BIT BIT_t;

#endif /* _BIT_H */
