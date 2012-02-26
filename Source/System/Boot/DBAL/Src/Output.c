/* 
 * Contains common Output definitions.
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

#include <stdint.h>
#include <String.h>
#include <Output.h>
#include <BootFiles.h>
#include <BIT.h>
#include <PMM.h>
#include <Log.h>
#include <Math.h>
#include <Macros.h>
  
// Switches to a video mode.
// uint32_t Mode                      The identifier for the mode we are about to switch to.
// VBEModeInfo_t *VBEMode             A pointer to the Mode Info structure.
//     rc
//                                    uint16_t - the status of the switch.
//                                             - non zero values indicate errors.
static uint16_t SwitchToMode(uint32_t Mode, VBEModeInfo_t *ModeInfo)
{
  	// The 8th bit specifies whether it's a VBE mode or not.
    if(Mode & (1 << 8))
    {
        // Switch to the mode, and save the status value.
        uint16_t Return = (uint16_t)BIT.Video.VideoAPI(VIDEO_VBE_SWITCH_MODE, (uint16_t)Mode);
        
        // If the mode is 256 colors, then set up the palette.
        if((ModeInfo->BitsPerPixel == 8) && (Return == 0x4F))
        { 
            // If the flag is zero, then it is VGA compatible.
            // Use the VGA function to setup the palette then.
            // Else the VBE one.
            if(!(ModeInfo->ModeAttributes & VGA_COMPATIBLE))
                BIT.Video.VideoAPI(VIDEO_VGA_PALETTE);

            else 
                BIT.Video.VideoAPI(VIDEO_VBE_PALETTE);
        }
           
        // Get rid of 0x4F - signifying function exists.
        Return &= ~0x4F;
        return Return;
    }
    
    else 
    {
        // Switch to the mode.
        BIT.Video.VideoAPI(VIDEO_VGA_SWITCH_MODE, (uint16_t)Mode);
   
        // If the mode is 256 colors, then set up the palette.
        if(ModeInfo->BitsPerPixel == 8)
            // Setup the palette to a RGB thingy.
	          BIT.Video.VideoAPI(VIDEO_VGA_PALETTE);
	        
	      return 0x0000;
    }
}

// Detects the "first" working serial port.
//     rc
//                                    uint32_t - the I/O address of the serial port.
static uint32_t SerialPortDetect()
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

// Get the best mode from VBEModeInfo[] array using all factors neccessary.
static VBEModeInfo_t FindBestVBEMode()
{ 
    // No comments now - this is going under a major refactor.
    VBEModeInfo_t Best = BIT.Video.VBEModeInfo[0];
	  Best.Score = fyl2x(Best.BitsPerPixel, BPP_SCALING_FACTOR);
	  Best.Score = sqrt((Best.Score * Best.Score) + 
	                    (Best.XResolution * Best.XResolution) +
	                    (Best.YResolution * Best.YResolution));
    
    // Calculate the score for each mode - while side by side, finding the best mode.
	  for(uint32_t i = 1; i < BIT.Video.VBEModeInfoN; i++)
	  {
        BIT.Video.VBEModeInfo[i].Score = fyl2x(Best.BitsPerPixel, BPP_SCALING_FACTOR);
	      BIT.Video.VBEModeInfo[i].Score = sqrt((Best.Score * Best.Score) + 
	                                            (Best.XResolution * Best.XResolution) +
	                                            (Best.YResolution * Best.YResolution));
	 
	      // If the score of this is greater than the best till now,
	      // make it the best.                                     
	      if(BIT.Video.VBEModeInfo[i].Score > Best.Score)
        {
	          Best = BIT.Video.VBEModeInfo[i];
        }
    }
    
	  return Best;
}

static EDIDModeInfo_t EDIDModeInfo[29];
static uint32_t EDIDModeInfoN;

// Parses the BIT.Video.EDIDInfo structure, cleaning it out, and making a usable
// array out of it.
static void ParseEDIDInfo()
{
    // Ok - so the most sensible thing to do would be to parse all the timings
    // specified in Established timings, Standard timings and Detalied timings
    // and put all the usable modes in a array consisting of the Horizontal resolution,
    // Vertical resolution and Refresh rate.
    
    // Later, this could be used to find out how many timings are supported of the 
    // "standard" (no - not the EDID standard, the standard standard) timings of 
    // a mode, adding to the score.

    // Currently, the number of EDIDModeInfo structures are 0.
    EDIDModeInfoN = 0;

    // Take care of the established modes.
    for(uint32_t i = 0; i < 2; i++)
    {
        for(uint32_t j = 0; j < 8; j++)
        {
            if(BIT.Video.EDIDInfo.EstablishedTimings[i] &
               (1 << j))
            {
                switch(i)
                {
                  // Handle established timings in byte 0 in EDIDInfo.EstablishedTimings.
                  case 0:
                  {  
                    switch(j)
                    {
                      // Bit 1.
                      case 0:
                        EDIDModeInfo[EDIDModeInfoN].XRes = 800;
                        EDIDModeInfo[EDIDModeInfoN].YRes = 600;
                        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 60;

                        EDIDModeInfoN++;
                        break;

                      // Bit 2.
                      case 1:
                        EDIDModeInfo[EDIDModeInfoN].XRes = 800;
                        EDIDModeInfo[EDIDModeInfoN].YRes = 600;
                        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 56;

                        EDIDModeInfoN++;
                        break;
                    
                      // Bit 3.
                      case 2:
                        EDIDModeInfo[EDIDModeInfoN].XRes = 640;
                        EDIDModeInfo[EDIDModeInfoN].YRes = 480;
                        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 75;

                        EDIDModeInfoN++;
                        break;

                      // Bit 4.
                      case 3:
                        EDIDModeInfo[EDIDModeInfoN].XRes = 640;
                        EDIDModeInfo[EDIDModeInfoN].YRes = 480;
                        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 72;
                        
                        EDIDModeInfoN++;
                        break;

                      // Bit 5.
                      case 4:
                        EDIDModeInfo[EDIDModeInfoN].XRes = 640;
                        EDIDModeInfo[EDIDModeInfoN].YRes = 480;
                        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 67;
                        
                        EDIDModeInfoN++;
                        break;
                        
                      // Bit 6.
                      case 5:
                        EDIDModeInfo[EDIDModeInfoN].XRes = 640;
                        EDIDModeInfo[EDIDModeInfoN].YRes = 480;
                        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 60;
                        
                        EDIDModeInfoN++;
                        break;

                      // Bit 7.
                      case 6:
                        EDIDModeInfo[EDIDModeInfoN].XRes = 720;
                        EDIDModeInfo[EDIDModeInfoN].YRes = 400;
                        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 88;
                        
                        EDIDModeInfoN++;
                        break;

                      // Bit 8.
                      case 7:
                        EDIDModeInfo[EDIDModeInfoN].XRes = 720;
                        EDIDModeInfo[EDIDModeInfoN].YRes = 400;
                        EDIDModeInfo[EDIDModeInfoN].RefreshRate = 70;
                        
                        EDIDModeInfoN++;
                        break;
                    }

                    break;
                  }

                  // Handle established timings in byte 1 in EDIDInfo.EstablishedTimings.
                  case 1:
                  {
                    switch(j)
                    {
                      // Bit 1.
                      case 0:

                        EDIDModeInfoN++;
                        break;

                      // Bit 2.
                      case 1:

                        EDIDModeInfoN++;
                        break;
                    
                      // Bit 3.
                      case 2:

                        EDIDModeInfoN++;
                        break;

                      // Bit 4.
                      case 3:

                        EDIDModeInfoN++;
                        break;

                      // Bit 5.
                      case 4:

                        EDIDModeInfoN++;
                        break;
                        
                      // Bit 6.
                      case 5:

                        EDIDModeInfoN++;
                        break;

                      // Bit 7.
                      case 6:

                        EDIDModeInfoN++;
                        break;

                      // Bit 8.
                      case 7:
                       
                        EDIDModeInfoN++;
                        break;
                    }

                    break;
                  }
                }
            }
        }
    }

    // Handle established timings in byte 2 in EDIDInfo.EstablishedTimings.
    if(BIT.Video.EDIDInfo.EstablishedTimings[2] & (1 << 7))
    {
      
    }

    // NOTE: The rest bits in byte 2 are reserved (for OEM use and other such stuff) - so..
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
        
        // Default fill the PhysBasePtr entry, in case it's not filled.
        if(!VBEModeInfo->PhysBasePtr)
        {
    		    VBEModeInfo->PhysBasePtr = 0xA0000;
		    }
		
        // If version is greater than equal to 0x0300, then replace all banked fields with linear fields.
        if(BIT.Video.VBECntrlrInfo->Version >= 0x0300)
        {
            // Replace all * fields with Lin* fields.
            VBEModeInfo->BytesPerScanLine = VBEModeInfo->LinBytesPerScanLine;
            memcpy(&VBEModeInfo->RedMaskSize, &VBEModeInfo->LinRedMaskSize, sizeof(uint8_t) * 8);
        }
        
        // Remove the entry if:
        if(
           // The current hardware configuration doesn't support the mode.
           !(VBEModeInfo->ModeAttributes & HARDWARE_INIT)                    ||
        
           // The mode is a text mode.
           !(VBEModeInfo->ModeAttributes & GRAPHICAL_MODE)                   ||

           // If it takes more than a bank, and LFB isn't available.
           ((((VBEModeInfo->XResolution * VBEModeInfo->YResolution * 
               VBEModeInfo->BitsPerPixel) / 8) > 0x10000) && 
           !(VBEModeInfo->ModeAttributes & LFB_AVAILABLE))                   ||
           
           // If the mode isn't 4-bpp, 8-bpp, 15-bpp, 16-bpp, 24-bpp and 32-bpp.
           ((VBEModeInfo->BitsPerPixel != 4)       &&
            (VBEModeInfo->BitsPerPixel != 8)       &&
            (VBEModeInfo->BitsPerPixel != 15)      &&
            (VBEModeInfo->BitsPerPixel != 16)      &&
            (VBEModeInfo->BitsPerPixel != 24)      &&
            (VBEModeInfo->BitsPerPixel != 32))                               ||
            
           // The mode if 4-bpp, and isn't VGA compatible.
           ((VBEModeInfo->BitsPerPixel == 4)       &&
            !(VBEModeInfo->ModeAttributes & VGA_COMPATIBLE))                 ||

           // The mode is smaller than equal to 320*200 and VGA exists.
           (((VBEModeInfo->XResolution <= 320)     &&
             (VBEModeInfo->YResolution <= 200))    &&
             (BIT.Video.VideoFlags & VGA_PRESENT))                           ||

           // The size of each scan line isn't dword divisible.
           (VBEModeInfo->BytesPerScanLine % 4)                               ||

           // The X resolution isn't divisible by 4.
           (VBEModeInfo->XResolution % 4)                                    ||

           // The memory model isn't:
           //     Packed Pixel.
           //     Direct Color.
           ((VBEModeInfo->MemoryModel != 4)        && 
            (VBEModeInfo->MemoryModel != 6))                                 ||
                  
           // Number of planes isn't 1 or 4 (for 4-bpp)
           ((VBEModeInfo->NumberOfPlanes  != 1)    ||
            ((VBEModeInfo->BitsPerPixel   == 4)    &&
             (VBEModeInfo->NumberOfPlanes != 4)))                            || 
                  
           ((VBEModeInfo->BitsPerPixel == 15) &&
            VERIFY_RGB_MODE(1, 15, 5, 10, 5, 5, 5, 0))                       ||
         
           ((VBEModeInfo->BitsPerPixel == 16) &&
            VERIFY_RGB_MODE(0, 0, 5, 11, 6, 5, 5, 0))                        ||
         
           ((VBEModeInfo->BitsPerPixel == 24) &&                       
            VERIFY_RGB_MODE(0, 0, 8, 16, 8, 8, 8, 0))                        ||
         
           ((VBEModeInfo->BitsPerPixel == 32) && 
            ((VERIFY_RGB_MODE(8, 24, 8, 16, 8, 8, 8, 0)                      ||
            (VERIFY_RGB_MODE(0,  0, 8, 16, 8, 8, 8, 0))))))
        {
		        // Move the required number of entries from i + 1 to i - effectively deleting the current entry.
            memmove(VBEModeInfo, &VBEModeInfo[1], 
                    sizeof(VBEModeInfo_t) * (BIT.Video.VBEModeInfoN - (i + 1)));
            BIT.Video.VBEModeInfoN--;
            
            // Here, we don't know whether the next entry is usable or not. So, move to previous, and continue.
            i--;
            continue;
        }
        
        // I've found out that certain video cards for 32-bpp modes set the 
        // Rsvd* fields to 0. I'm not too sure why - but we'll just set them to
        // the default value we want them to be.
        if(VBEModeInfo->BitsPerPixel == 32)
        {
            VBEModeInfo->RsvdFieldPosition = 24;
            VBEModeInfo->RsvdMaskSize = 8;
        }

        if(VBEModeInfo->ModeAttributes & LFB_AVAILABLE)
        {
            // Set the use LFB bit.
            VBEModeInfo->Mode |= (1 << 14);
        }

        VBEModeInfo->BytesBetweenLines = (VBEModeInfo->BytesPerScanLine) - 
                                          ((VBEModeInfo->XResolution * VBEModeInfo->BitsPerPixel) / 8);
    
        if(VBEModeInfo->BitsPerPixel == 15)
        {
            VBEModeInfo->BytesBetweenLines -= VBEModeInfo->XResolution / 8;
        }
    }

    return;
}

/*
 * Initializes VGA for a graphical color mode.
 */
static void VGAInit()
{ 
    // Fill in some general details of the video mode.
    BIT.Video.ModeInfo.Mode = MODE_640_480_16;
    BIT.Video.ModeInfo.PhysBasePtr = 0xA0000;
    BIT.Video.ModeInfo.XResolution = 640;
    BIT.Video.ModeInfo.YResolution = 480;

    // Set the bits per pixel and the number of planes for the mode.
    BIT.Video.ModeInfo.BitsPerPixel = 4;
    BIT.Video.ModeInfo.NumberOfPlanes = 4;
    
    // The bytes between lines and the bytes per scan line.
    BIT.Video.ModeInfo.BytesBetweenLines = 0;
    BIT.Video.ModeInfo.BytesPerScanLine = (640 * 4) / 8;
    
    // And the mode is supported by hardware, is a graphical mode and is VGA compatible.
    BIT.Video.ModeInfo.ModeAttributes = HARDWARE_INIT | GRAPHICAL_MODE | VGA_COMPATIBLE;  

    BIT.Video.VideoFlags |= GRAPHICAL_USED;
    
    // Go to the 320*200*256 colors mode.
    SwitchToMode(MODE_640_480_16, &BIT.Video.ModeInfo);    
}
      
/*  
 * Initializes VBE for a graphical mode.
 * If something fails, automatically reverts to VGA.
 */
static void VBEInit()
{   
    // If mode number is below 0x0102, then, revert. 
    if(BIT.Video.VBECntrlrInfo->Version < 0x0102)
    {
        // TODO: Implement this.
        //OutputRevert();
        return;
    }

    // Get the segment and the offset of the list of the modes.
    uint16_t Segment = BIT.Video.VBECntrlrInfo->VideoModesFar & 0xFFFF0000;
    uint16_t Offset  = BIT.Video.VBECntrlrInfo->VideoModesFar & 0x0000FFFF;
        
    // Make flat pointer, from segment and offset = (segment * 0x10) + offset;
    uint16_t *VideoModesFlat = (uint16_t*)((Segment * 0x10) + Offset);
    uint16_t Mode = *VideoModesFlat++;
    uint32_t Entries = 0;
        
    // Keep looping till we reach the End of Entries.
    do
    {
        // So we got one more entry.
        Entries++;
        // Get the mode into Mode, and move on to the next video mode.
        Mode = *VideoModesFlat++;
    } while(Mode != 0xFFFF);
        
    // Allocate some memory from the Base Bitmap to hold all the mode information.
    BIT.Video.VBEModeInfo = (VBEModeInfo_t*)PMMAllocContigFrames(BASE_BITMAP, 
                            ((sizeof(VBEModeInfo_t) * Entries) + 0xFFF) / 0x1000);

    // If we failed to allocate enough space, simply revert back.
    if(!BIT.Video.VBEModeInfo)
    {
        // TODO: Implement this.
        //OutputReturn();      
        return;
    }
    
    // Get mode information from VBE, and the number of entries in VBEModeInfoN.
    BIT.Video.VBEModeInfoN = BIT.Video.VideoAPI(VIDEO_VBE_GET_MODES, BIT.Video.VBEModeInfo);
    
    // Parse the VBEModeInfo[] array, and clean it out for usable modes.
    ParseVBEInfo();
    
    // If no mode was defined "usable", then revert.
    if(!BIT.Video.VBEModeInfoN)
    {
        // TODO: Implement this.
        //OutputRevert();
        return;
    }

    // Parse EDID Information.
    ParseEDIDInfo();

    // Get the best mode from VBEModeInfo[] array - and then switch to it.
    BIT.Video.ModeInfo = FindBestVBEMode();
        
    BIT.Video.VideoFlags |= GRAPHICAL_USED;

    // Switch to the mode.
    if(SwitchToMode(BIT.Video.ModeInfo.Mode, &BIT.Video.ModeInfo))
    {
        // If the mode switch was unsuccessful.
        // TODO: Implement this.
        //OutputRevert();

        return;
    }
}

// Initializes the first available serial port.
static void SerialInit()
{
    // Find the "first" working serial port, and get it's address.
    BIT.Serial.Port = SerialPortDetect();  
        
    if(BIT.Serial.Port)
        BIT.Serial.SerialFlags = SERIAL_USED;    
}

/*
 * Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
 * If no video card, initializes the serial port.
 */
void OutputInit()
{
    // If VBE is present, initialize VBE for a graphical mode.
    if(BIT.Video.VideoFlags & VBE_PRESENT)
        VBEInit();
        
    // Else, if VGA is present, initialize VGA for a graphical mode.
    else if(BIT.Video.VideoFlags & VGA_PRESENT)
        VGAInit();

    // Else, initialize the serial port (if found).
    else
        SerialInit();
    
    // If we are using a graphical mode, then load the background image.
    if(BIT.Video.VideoFlags & GRAPHICAL_USED)
    {
        // Open the file, and get it's information.
        BIT.Video.BackgroundImg = BootFilesBGImg();
    }
}
