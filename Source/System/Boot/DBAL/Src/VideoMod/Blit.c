/* Contains common definitions to output to the screen.
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
#include <VideoMod/Blit.h>
#include <BIT.h>
#include <PMM.h>
#include <String.h>

extern uint32_t *OldBuffer;

// Gives a buffer, of bpp being what we require, to be blitted to the screen.
// uint32_t *Buffer                   The address of the buffer to print.
void BlitBuffer(uint32_t *Buffer)
{
    if(BIT.Video.BPP == 4)
        BlitBuffer4BPP(Buffer);
}

/*
void Init()
{
    OldBuffer = (uint32_t*)PMMAllocContigFrames(38);
    memset(OldBuffer, 0, 38 * 0x1000);   
} */
