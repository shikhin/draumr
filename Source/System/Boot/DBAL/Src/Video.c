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

#include <stdint.h>
#include <Video.h>
#include <BIT.h>
#include <PMM.h>
#include <Log.h>

// A pointer to video information by VBE. 
VideoInfoVBE_t *VideoInfoVBE;
ModeInfoVBE_t *ModeInfoVBE;

// Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
void VideoInit()
{
    // If VBE was present, use a VBE mode.
    if(BIT.VideoFlags & VBE_PRESENT)    
    {
        VideoInfoVBE = (VideoInfoVBE_t*)BIT.VideoInfo;
	 
	// Get an approximate size of the buffer.
	uint32_t BufferSize = VideoInfoVBE->Entries * sizeof(ModeInfoVBE_t);
	BufferSize = (BufferSize + 0xFFF) & 0xFFF;
	 
	// And allocate pages for it - and get the mode information into the buffer.
	uint32_t Buffer = PMMAllocContigFrames(BufferSize/0x1000);

	BIT.VBEGetModeInfo(Buffer);
        ModeInfoVBE = (ModeInfoVBE_t*)Buffer;
    }
}