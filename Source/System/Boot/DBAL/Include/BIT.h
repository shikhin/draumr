/* General BIT related definitions and structures.
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

// Serial flags.
#define SERIAL_USED     (1 << 0)

// Boot device flags.
#define BD_CD           0
#define BD_FLOPPY       1
#define BD_PXE          2

// The BIT structure.
struct BIT
{
    // The open, read and close file functions.
    uint32_t (*OpenFile)(uint32_t FileCode);
    void     (*ReadFile)(void *Destination, uint32_t Bytes);
    void     (*CloseFile)();
    
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
        
        void     (*SwitchVGA)(uint16_t Mode);        // The function which performs the switch to a vga mode.
	void     (*SetupPaletteVGA)();               // The function which sets up a palette for a 8bpp mode.
	uint32_t (*GetModeInfoVBE)(VBEModeInfo_t *); // The function which gets Video mode information from VBE.
	
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
extern volatile BIT_t BIT;

// Initializes the BIT structure, copying it to somewhere appropriate.
// uint32_t *BITPointer               The pointer to the BIT structure, as passed to us.
void BITInit(uint32_t *BITPointer);

#endif            /* BIT.h */
