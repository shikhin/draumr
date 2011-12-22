/* Contains common Ouput definitions.
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

#include <stdint.h>
#include <String.h>
#include <Output.h>
#include <BootFiles.h>
#include <BIT.h>
#include <PMM.h>
#include <Log.h>

// Switches to a video mode.
// uint32_t Mode                      The identifier for the mode we are about to switch to.
static void SwitchToMode(uint32_t Mode)
{
    if(BIT.Video.VideoFlags & VBE_PRESENT)
    {

    }
    
    else 
    {
        // Switch to the mode.
        BIT.Video.SwitchVGA((uint16_t)Mode);
   
        // If the mode is 256 colors, then set up the palette.
        if(BIT.Video.BPP == 8)
            // Setup the palette to a RGB thingy.
	        BIT.Video.SetupPaletteVGA();
    }
}

// Detects the "first" working serial port.
//     rc
//                                    uint32_t - the I/O address of the serial port.
static uint32_t DetectSerialPort()
{
    // Define the IO port for each of the COM port.
    static uint16_t Ports[4] = { 0x3F8,
                                 0x2F8,
                				 0x3E8,
				                 0x2E8 };
				 
    
    // Loop through the each of the port.
    for(uint32_t i = 0; i < 4; i++)
    {
      	// Offset 7 is the scratch register.
	    // Write 0xA5 (arbitary) to it, and check if we can read 0xA5 back.
	    outb(Ports[i] + 7, 0xA5);
	
	    // If we can't read 0xA5 back, then continue (assume this one's faulty or something).
	    if(inb(Ports[i] + 7) != 0xA5)
	         continue;
      
        // At offset 1 is the Interrupt Enable register.
        // Disable all interrupts for this one.
        outb(Ports[i] + 1, 0x00);
	 
        // At offset 3 is the Line Control register.
 	    // Set the Divisor Latch Access Bit - so we set the Divisor next time.
        outb(Ports[i] + 3, 0x80);
	
	    // Offset 0 and 1 is the divisor register too.
	    // Set the divisor to 3, which comes out to a frequency of 38400 baud.
	    outb(Ports[i] + 0, 0x03);
	    outb(Ports[i] + 1, 0x00);
	
   	    // Offset 3 is the Line Control register.
	    // Set it to 8 bits, 1 stop, no parity.
	    outb(Ports[i] + 3, 0x03);
	
	    // Offset 4 is the Modem Control Register.
	    // 0x13 sets the Loobpack mode, the Request to Send and the Data Terminal Ready bits.
	    outb(Ports[i] + 4, 0x13);	

	    uint32_t Timeout = 0;
	
	    // Read from the Line status register.
	    // And keep looping till the transmit register isn't empty.
	    // And, till Timeout < 1million. If we reach 1million first, then faulty port.
	    while((!(inb(Ports[i] + 5) & 0x20))
	          && (Timeout < MIN_TIMEOUT))
	    {
	        __asm__ __volatile__("pause");
	        Timeout++;
	    }
	
	    // If we reached timeout, the continue and assume faulty.
	    if(Timeout == MIN_TIMEOUT)
	        continue;
	
   	    // Output 0xA5 - now, since we are in loopback mode, the input register would also have 0xA5.
	    outb(Ports[i], 0xA5);
	
	    Timeout = 0;
	    // And keep looping till we didn't recieve something.
	    // And, till Timeout < 1million. If we reach 1million first, then faulty port.
	    while((!(inb(Ports[i] + 5) & 0x01))
	           && (Timeout < MIN_TIMEOUT))
	    {  
	        __asm__ __volatile__("pause");
	        Timeout++;
   	    }
	
	    // If we reached timeout, the continue and assume faulty.
	    if(Timeout == MIN_TIMEOUT)
	        continue;
	
  	    // If not, assume faulty or something.
	    if(inb(Ports[i]) != 0xA5)
	        continue;
	
	    // Offset 4 is the Modem Control Register.
	    // 0x3 sets the Request to Send and the Data Terminal Ready bits.
	    outb(Ports[i] + 4, 0x3);
	    return Ports[i];
    }
    
    return 0x0000;
}

// A lookup table for text modes, for VBE < 1.2
static uint16_t TextModeLookupTable[5][2] = 
{
    { 80 , 60 },
    { 132, 25 },
    { 132, 43 },
    { 132, 50 },
    { 132, 60 }
};

// Fill information about available text modes - only to be used if VBE Version < 1.2
static void FillTextInfoVBE()
{
    for(uint32_t i = 0; i < BIT.Video.VBEModeInfoN; i++)
    {
        // If:
        //      The mode cannot be initialized in the present hardware configuration.
        //      It isn't a text mode.
        //      It doesn't lie in the "standard VESA defined" text mode range (0x108 - 0x10C).
        // Then, remove the entry.
        if(!(BIT.Video.VBEModeInfo[i].ModeAttributes & HARDWARE_INIT) ||    // Can/cannot be initialized. 
           (BIT.Video.VBEModeInfo[i].ModeAttributes & GRAPHICAL_MODE) ||     // Text mode/graphics mode.
           (BIT.Video.VBEModeInfo[i].Mode < 0x108) ||                  // In the text mode range.
           (BIT.Video.VBEModeInfo[i].Mode > 0x10C))
        {
            // Move the required number of entries from i + 1 to i - effectively deleting the current entry.
            memmove(&BIT.Video.VBEModeInfo[i], &BIT.Video.VBEModeInfo[i + 1], 
                    sizeof(VBEModeInfo_t) * (BIT.Video.VBEModeInfoN - (i + 1)));
            BIT.Video.VBEModeInfoN--;
            
            // Here, we don't know whether the next entry is usable or not. So, move to previous, and continue.
            i--;
            continue;
        }
        
        BIT.Video.VBEModeInfo[i].ModeAttributes = HARDWARE_INIT;
        BIT.Video.VBEModeInfo[i].PhysBasePtr = 0xB8000;         // Keep the base pointer as 0xB8000.
        BIT.Video.VBEModeInfo[i].NumberOfPlanes = 1;            // The number of planes as 1.
        BIT.Video.VBEModeInfo[i].BitsPerPixel = 16;             // And bits per pixel as 16.
        BIT.Video.VBEModeInfo[i].NumberOfBanks = 1;             // Don't know - as the VBE 3.0 core specification said.
        BIT.Video.VBEModeInfo[i].Reserved = 1;                  // This should be 1 - for a future feature.    
        
        // Put the appropriate resolution for the appropriate mode.
        // The lookup table starts at 0 - 4, for 0x108 - 0x10C - just minus 0x108 from the mode.
        BIT.Video.VBEModeInfo[i].XResolution = TextModeLookupTable[BIT.Video.VBEModeInfo[i].Mode - 0x108][0];
        BIT.Video.VBEModeInfo[i].YResolution = TextModeLookupTable[BIT.Video.VBEModeInfo[i].Mode - 0x108][1];
    }
    
    return;
}

// Parse the VBEModeInfo[] array, and clean it out for usable modes.
static void ParseVBEInfo()
{
	// For easier, accesses (rather than calculating [i] again and again). (though I think the compiler would optimize the previous one out anyway).
    VBEModeInfo_t *VBEModeInfo;
    // Check through each entry, removing unneccessary ones.
    for(uint32_t i = 0; i < BIT.Video.VBEModeInfoN; i++)
    {
        VBEModeInfo = &BIT.Video.VBEModeInfo[i];
        // If we haven't defined the PhysBasePtr - fill it.
        if(!VBEModeInfo->PhysBasePtr)
        {
		    if(VBEModeInfo->ModeAttributes & GRAPHICAL_MODE)
		        VBEModeInfo->PhysBasePtr = 0xA0000;
		        
		    else
		        VBEModeInfo->PhysBasePtr = 0xB8000;	
		}
		
        // If this is a text mode, continue with it, "as it is".
        if(!(VBEModeInfo->ModeAttributes & GRAPHICAL_MODE))
            continue;
        
           // Cannot be initialized in hardware -
        if(!(VBEModeInfo->ModeAttributes & HARDWARE_INIT) ||    
            // We are in a 4BPP mode -
           (VBEModeInfo->BitsPerPixel == 4) ||                  
           // It requires bank switching, and LFB isn't supported -
           ((((VBEModeInfo->XResolution * VBEModeInfo->YResolution * 
               VBEModeInfo->BitsPerPixel) / 8) > 0x20000) && 
            !(VBEModeInfo->ModeAttributes & LFB_AVAILABLE)))
        {
		    // Move the required number of entries from i + 1 to i - effectively deleting the current entry.
            memmove(VBEModeInfo, &VBEModeInfo[1], 
                    sizeof(VBEModeInfo_t) * (BIT.Video.VBEModeInfoN - (i + 1)));
            BIT.Video.VBEModeInfoN--;
            
            // Here, we don't know whether the next entry is usable or not. So, move to previous, and continue.
            i--;
            continue;
        }
        
        // If version is greater than equal to 0x0300, then replace all banked fields with linear fields.
        if(BIT.Video.VBECntrlrInfo->Version >= 0x0300)
        {
            // Replace all * fields with Lin* fields.
            VBEModeInfo->BytesPerScanLine = VBEModeInfo->LinBytesPerScanLine;
            memcpy(&VBEModeInfo->RedMaskSize, &VBEModeInfo->LinRedMaskSize, sizeof(uint8_t) * 8);
        }
        
        // Remove the entry if doesn't suit us..
        if((VBEModeInfo->BytesPerScanLine % 4) ||
           (VBEModeInfo->XResolution % 4)      ||

           ((VBEModeInfo->MemoryModel != 0)    &&
            (VBEModeInfo->MemoryModel != 4)    && 
            (VBEModeInfo->MemoryModel != 6))   ||
         
           ((VBEModeInfo->BitsPerPixel == 8) &&
            (!(VBEModeInfo->DirectColorModeInfo & COLOR_RAMP_PROGRAMMABLE))) ||
         
           ((VBEModeInfo->BitsPerPixel == 15) &&
            VERIFY_RGB_MODE(1, 15, 5, 10, 5, 5, 5, 0))                       ||
         
           ((VBEModeInfo->BitsPerPixel == 16) &&
            VERIFY_RGB_MODE(0, 0, 5, 11, 6, 5, 5, 0))                        ||
         
           ((VBEModeInfo->BitsPerPixel == 24) &&                       
            VERIFY_RGB_MODE(0, 0, 8, 16, 8, 8, 8, 0))                        ||
         
           ((VBEModeInfo->BitsPerPixel == 32) && 
            VERIFY_RGB_MODE(8, 24, 8, 16, 8, 8, 8, 0)))
        {
		    // Move the required number of entries from i + 1 to i - effectively deleting the current entry.
            memmove(VBEModeInfo, &VBEModeInfo[1], 
                    sizeof(VBEModeInfo_t) * (BIT.Video.VBEModeInfoN - (i + 1)));
            BIT.Video.VBEModeInfoN--;
            
            // Here, we don't know whether the next entry is usable or not. So, move to previous, and continue.
            i--;
            continue;
        }
    }

    return;
}

// Initializes VGA for a 320*200*256 color mode.
static void InitVGA()
{ 
    // Fill in some general details of the video mode.
    BIT.Video.Address = (uint32_t*)0xA0000;
    BIT.Video.XRes = 320;
    BIT.Video.YRes = 200;
    BIT.Video.BPP = 8;
    BIT.Video.BytesBetweenLines = 0;
    BIT.Video.VideoFlags |= GRAPHICAL_USED;
    
    // Go to the 320*200*256 colors mode.
    SwitchToMode(0x13);    
}
        
// Initializes VBE for a graphical mode.
// If something fails, automatically reverts to VGA.
static void InitVBE()
{    
    // Get the segment and the offset.
    uint16_t Segment = BIT.Video.VBECntrlrInfo->VideoModesFar & 0xFFFF0000;
    uint16_t Offset = BIT.Video.VBECntrlrInfo->VideoModesFar & 0x0000FFFF;
        
    // Flat pointer, from segment and offset = (segment * 0x10) + offset;
    uint16_t *VideoModesFlat = (uint16_t*)((Segment * 0x10) + Offset);
    uint16_t Mode = *VideoModesFlat++;
    uint32_t Entries = 0;
        
    // Keep looping till we reach the End of Entries thingy.
    do
    {
        // So we got one more entry.
        Entries++;
        // Get the mode into Mode, and move on to the next video mode thingy.
        Mode = *VideoModesFlat++;
    } while(Mode != 0xFFFF);
        
    // Allocate some memory from the Base Stack to hold all the mode information.
    BIT.Video.VBEModeInfo = (VBEModeInfo_t*)PMMAllocContigFrames(BASE_STACK, 
                            ((sizeof(VBEModeInfo_t) * Entries) + 0xFFF) / 0x1000);
        
    // If we failed to allocate enough space, simply revert to VGA.
    if(!BIT.Video.VBEModeInfo)
    {
        InitVGA();      
        return;
    }
    
    // If version number is 1.0, zero out the array. 
    if(BIT.Video.VBECntrlrInfo->Version == 0x0100)
        memset(BIT.Video.VBEModeInfo, 0, (sizeof(VBEModeInfo_t) * Entries));
    
    // Get mode information from VBE, and the number of entries in VBEModeInfoN.
    BIT.Video.VBEModeInfoN = BIT.Video.GetModeInfoVBE(BIT.Video.VBEModeInfo);

    // If version number is below 1.2, then, certainly, only text modes would be available.
    // Thus, fill all the text mode information in VBEModeInfo[], and revert to VGA.
    if(BIT.Video.VBECntrlrInfo->Version < 0x0102)
    {
        // Fill information about available text modes.
        FillTextInfoVBE();    
        InitVGA();
        return;
    }
    
    // Parse the VBEModeInfo[] array, and clean it out for usable modes.
    ParseVBEInfo();
}

// Initializes the first available serial port.
static void InitSerial()
{
    // Find the "first" working serial port, and get it's address.
    BIT.Serial.Port = DetectSerialPort();  
        
    if(BIT.Serial.Port)
        BIT.Serial.SerialFlags = SERIAL_USED;    
}

// Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
void OutputInit()
{
    // If VBE is present, initialize VBE for a graphical mode.
    // If something fails in there, it automatically reverts t VGA.
    if(BIT.Video.VideoFlags & VBE_PRESENT)
        InitVBE();
        
    // Else If VGA is present initialize VGA for a 320*200*256 color mode.
    else if(BIT.Video.VideoFlags & VGA_PRESENT)
        InitVGA();

    // Initialize the serial port thingy.
    else
        InitSerial();
    
    // If we are using a graphical mode, then load the background image.
    if(BIT.Video.VideoFlags & GRAPHICAL_USED)
    {
        // Open the file, and get it's information.
        BIT.Video.BackgroundImg = BootFilesBGImg();
    }
}
