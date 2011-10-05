/* Contains common Video definitions.
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

// Video Info for VBE.
struct VideoInfoVBE
{
    uint32_t Entries;                 // The number of video modes.
    uint32_t EntriesA;                // The number of accelerated video modes.
    
    uint8_t  VBEControllerInfo[512];  // The controller information, given to us by VBE.
} __attribute__((packed));

// Mode Info for VBE
struct ModeInfoVBE
{    
    uint16_t Mode;
    uint16_t Attributes;
    uint8_t  WinA, WinB;
    uint16_t Granularity;
    uint16_t WinSize;
    uint16_t SegmentA, SegmentB;
    uint32_t WinFuncPtr;
    uint16_t Pitch; 
 
    uint16_t XRes, YRes;
    uint8_t  WChar, YChar, Planes, BPP, Banks;
    uint8_t  MemoryModel, BankSize, ImagePages;
    uint8_t  Reserved0;
 
    uint8_t  RedMask, RedPosition;
    uint8_t  GreenMask, GreenPosition;
    uint8_t  BlueMask, BluePosition;
    uint8_t  RsvMask, RsvPosition;
    uint8_t  DirectColorAttributes;
 
    uint32_t PhysBase;
    uint32_t Reserved1;
    uint16_t Reserved2;
    
    uint16_t LinBytesPerScanLine;
    uint8_t  BnkNumberOfImagePages;
    uint8_t  LinNumberOfImagePages;
    uint8_t  LinRedMaskSize;
    uint8_t  LinRedFieldPosition;
    uint8_t  LinGreenMaskSize;
    uint8_t  LinGreenFieldPosition;
    uint8_t  LinBlueMaskSize;
    uint8_t  LinBlueFieldPosition;
    uint8_t  LinRsvdMaskSize;
    uint8_t  LinRsvdFieldPosition;
    uint32_t MaxPixelClock;
    
    uint8_t  Reserved3[190];
    uint32_t Score;
} __attribute__((packed));
    
typedef struct VideoInfoVBE VideoInfoVBE_t; // Typedef it!
typedef struct ModeInfoVBE  ModeInfoVBE_t;  // Typedef it too. Comments overdose :P

// Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
void VideoInit();

#endif                                /* Video.h */