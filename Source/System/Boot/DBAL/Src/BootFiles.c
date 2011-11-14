/*  Definitions of functions to open, read and close boot files.
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

#include <BootFiles.h>
#include <stdint.h>
#include <PMM.h>
#include <Abort.h>
#include <BIT.h>
#include <Log.h>

void *Bouncer = (void*)0;
static uint32_t BouncerSize = BOUNCER_SIZE;

// Initializes the bouncer in which we would be reading the required boot files.
void InitBootFiles()
{
    // Try to allocate the big size - recommended.
    Bouncer = (void*)PMMAllocContigFrames(BASE_STACK, BOUNCER_PAGES);
    if(!Bouncer)
    {
        // If the failed, try to allocate small size.
        Bouncer = (void*)PMMAllocContigFrames(BASE_STACK, BOUNCER_SMALL_PAGES);
        if(!Bouncer)
            AbortBoot("ERROR: Can't allocate enough pages for bouncer.\n");
        
        BouncerSize = BOUNCER_SMALL_SIZE;
    }
}

// Clears up everything initialized in the Init().
void ClearBootFiles()
{
    // TODO: Implement this.
    //PMMFreeContigFrames(BASE_STACK, Bouncer, BOUNCER_PAGES);
}

// Gets the background image, verifying what we are getting to.
// void *Location                     Pointer to a pointer where we store the image.
//     rc
//                                    uint32_t - the length of the file.
uint32_t BootFilesBGImg(uint32_t **Location)
{
    Location = Location;
    uint32_t Size = BIT.OpenFile(0x02);
    if(!Size)
        return 0;
    
    // Read 512 bytes at the bouncer
    BIT.ReadFile((uint32_t*)Bouncer, 512);
    
    uint8_t *Signature = (uint8_t*)Bouncer;
    // So the file isn't valid.
    if((Signature[0] != 'B') ||
       (Signature[1] != 'M'))
        return 0;    
    
    *Location = (uint32_t*)PMMAllocContigFrames(POOL_STACK, (Size + 0xFFF)/0x1000);
    // So we can't allocate space for the image.
    if(!(*Location))
        return 0;
    
    return Size;
}
