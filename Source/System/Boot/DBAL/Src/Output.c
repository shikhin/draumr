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

	// If the mode is 640 * 480 * 16 colors, then set up the required bit map and RMSR.
        if(Mode == 0x12)
        {
            // Set the bit mask - 0xFF - we except everything!
	    outb(0x03CE, 0x08);
	    outb(0x03CF, 0xFF);
	
            // Set the address register to a) The Write Map Select Register b) The Read Map Select Register.
            outb(0x03C4, 0x02);
            outb(0x03CE, 0x04);
        }
    
        // If the mode is 320 * 200 * 256 colors, then set up the palette.
        else if(Mode == 0x13)
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
	// Write 0xA5 (arbitary) to it, and check if we can read 0x2A back.
	outb(Ports[i] + 7, 0xA5);
	
	// If we can't read 0xA5 back, then continue (assume this one's faulty or something.
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

// Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
void OutputInit()
{
    // If VBE is present, find the best VBE mode, and use it.
    if(BIT.Video.VideoFlags & VBE_PRESENT)
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
	VBEModeInfo = (VBEModeInfo_t*)PMMAllocContigFrames(BASE_STACK, ((sizeof(VBEModeInfo_t) * Entries) + 0xFFF) / 0x1000);
    
        // Get mode information from VBE.
	BIT.Video.GetModeInfoVBE();
    }
    
    // If VGA is present, then use 320*200*256 color mode (or 640*480*16 colors).
    else if(BIT.Video.VideoFlags & VGA_PRESENT)
    {
        // If high resolution is set, then switch to 640*480*16 colors mode.
        #ifdef HIGH_RESOLUTION
        SwitchToMode(0x12);
	
	// Fill in some general details of the video mode.
	BIT.Video.Address = (uint32_t*)0xA0000;
	BIT.Video.XRes = 640;
	BIT.Video.YRes = 480;
	BIT.Video.BPP = 4;
	BIT.Video.BytesBetweenLines = 0;
	
	// Else, go to the 320*200*256 colors mode.
        #else  
        SwitchToMode(0x13);
    
        // Fill in some general details of the video mode.
        BIT.Video.Address = (uint32_t*)0xA0000;
        BIT.Video.XRes = 320;
        BIT.Video.YRes = 200;
        BIT.Video.BPP = 8;
        BIT.Video.BytesBetweenLines = 0;
        
	#endif    /* HIGH_RESOLUTION */ 
    }

    // Initialize the serial port thingy.
    else
    {
        // Find the "first" working serial port, and get it's address.
        BIT.Serial.Port = DetectSerialPort();  
	
	if(BIT.Serial.Port)
	    BIT.Serial.SerialFlags = SERIAL_PRESENT;
    }
}