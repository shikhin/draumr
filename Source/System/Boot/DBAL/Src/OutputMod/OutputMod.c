/* Contains definitions for the output module.
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
#include <OutputMod/OutputMod.h>
#include <BIT.h>
#include <PMM.h>
#include <String.h>
#include <Log.h>
#include <OutputMod/Enhance.h>
#include <OutputMod/Blit.h>

// The old buffer, used for comparision to allow unneccessary copying to video memory.
uint32_t *OldBuffer = (uint32_t*)0;

// The buffer where we draw everything. If allocating for this fails, then we go to standard text mode.
uint32_t *DrawBoard = (uint32_t*)0;

// The temporary buffer, where we dither. If allocating this fails, then we go to standard text mode.
uint32_t *TempBuffer = (uint32_t*)0;

// The temporary buffer, where we keep the error for the next line.
uint8_t  *TempErrorLine = (uint8_t*)0;

// Initializes the output module, allocating neccessary buffers and such.
void OutputModInit()
{
    // If VGA or VBE is present, then, prepare for it.
    if(BIT.Video.VideoFlags & GRAPHICAL_USED)
    {
        BMPHeader_t *BMPHeader = (BMPHeader_t*)BIT.Video.BackgroundImg.Location;
        
        // We draw in 24BPP.
        uint32_t NoPages = (BIT.Video.XRes * BIT.Video.YRes * 24)/8;
        NoPages = (NoPages + 0xFFF)/0x1000;
        
        // If we can't allocate space for the "draw board", then gracefully return...
        // ...to text mode :P
        DrawBoard = (uint32_t*)PMMAllocContigFrames(POOL_STACK, NoPages);
        
        // 24BPP for the temporary buffer.
        // + 1 extra line for the temporary line buffer.
        NoPages = (BIT.Video.XRes * (BIT.Video.YRes + 1) * 24)/8;
        NoPages = (NoPages + 0xFFF)/0x1000;
        
        // If allocating that fails, then go to text mode.. 
        TempBuffer = (uint32_t*)PMMAllocContigFrames(POOL_STACK, NoPages);
        
        if(!DrawBoard || !TempBuffer)
        {
            // Free the image buffer.
            // TODO: Implement this.
            //PMMFreeContigFrames(BIT.Video.BackgroundImg, (BIT.Video.BackgroundImg.Size + 0xFFF)/0x1000);
            
            // TODO: Implement this.
            // Revert back to a usable text mode.
            //BIT.Video.Revert();                                                                                             
            return;
        }
                
        // Clear the temporary buffer an draw board out.
        memset(TempBuffer, 0, NoPages * 0x1000);
        memset(DrawBoard, 0, (BIT.Video.XRes * BIT.Video.YRes * 24)/8);
        
        // Go to the last line for the temporary error line.
        TempErrorLine = (uint8_t*)TempBuffer + ((BIT.Video.XRes * BIT.Video.YRes * 24)/8);
        
        NoPages = (BIT.Video.XRes * BIT.Video.YRes * BIT.Video.BPP)/8;
        NoPages = (NoPages + 0xFFF)/0x1000;
        // If the call fails, then other blitting code realizes it, and directly blits.
        OldBuffer = (uint32_t*)PMMAllocContigFrames(POOL_STACK, NoPages);
        if(OldBuffer)
        {
            memset(OldBuffer, 0, NoPages * 0x1000);
        }
        
        // If we haven't opened the background image file, return.
        if(!BIT.Video.BackgroundImg.Size)
            return;
                
        // Resize the image to the scaled buffer.
        ResizeBilinear((uint8_t*)BIT.Video.BackgroundImg.Location + BMPHeader->Offset,
                       (uint8_t*)DrawBoard, BMPHeader->XSize, BMPHeader->YSize,
                       BIT.Video.XRes, BIT.Video.YRes);
        
        //TODO: Implement this.
        //PMMFreeContigFrames(BIT.Video.BackgroundImg.Location, (BIT.Video.BackgroundImg.Size + 0xFFF)/0x1000);
               
        // Blit the background image.
        BlitBuffer(DrawBoard);
    }
} 
