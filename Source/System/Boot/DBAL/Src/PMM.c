/* Contains common PMM definitions.
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
#include <PMM.h>
#include <BIT.h>
#include <Log.h>
#include <Abort.h>

// Define the pointers to the headers and entries.
MMapHeader_t *MMapHeader;
MMapEntry_t  *MMapEntries;

// The MAX macro, move it to it's own private place sometime.
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// The Top of the Pool stack, and the base memory stack - first is base, second is pool.
static uint32_t *Top[2];

// PMMFixMMap fixes the memory map - only overlapping entries.
static void PMMFixMMap()
{
    // Fix all entries not starting on page boundaries.
    for(uint16_t i = 0; i < MMapHeader->Entries; i++)
    {
        // And the length and the start address to the below page boundary.
        MMapEntries[i].Start = (MMapEntries[i].Start + 0xFFF) & ~0xFFF;
	MMapEntries[i].Length &= ~0xFFF;
	
        if((MMapEntries[i].Start > (MMapEntries[i].Start + MMapEntries[i].Length)) ||
           (MMapEntries[i].Length == 0))
        {
             // Move the required number of entries from i + 1 to i.
	    memmove(&MMapEntries[i], &MMapEntries[i + 1], 
		    sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 1)));
	    MMapHeader->Entries--;
	    
	    // Here, we don't know about the current entry, so, move to previous, and continue.
	    i--;
	    continue; 
        }
    }
        
    for(uint16_t i = 0; i < (MMapHeader->Entries - 1); i++)
    {
        // And fix all the shit here.
        // If the flags field isn't equal, then just continue - the flags can virtually make different entries.
        if(MMapEntries[i].Flags != MMapEntries[i + 1].Flags)
	    continue;
	
	// Get the recommended type, the maximum of the both. Since types are overriding, this should work..
	uint32_t Type = MAX(MMapEntries[i].Type, MMapEntries[i + 1].Type);
	
	// If they are the same start and length.
        // -------
	// -------
	if((MMapEntries[i].Start == MMapEntries[i + 1].Start) &&
	   (MMapEntries[i].Length == MMapEntries[i + 1].Length))
	{
	    // Move the required number of entries from i + 2 to i + 1.
	    memmove(&MMapEntries[i + 1], &MMapEntries[i + 2], 
		    sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 2)));
	    MMapEntries[i].Type = Type;
	    MMapHeader->Entries--;
	    
	    // Here, we don't know whether we overlap with the next entry or not. So, move to previous, and continue.
	    i--;
	    continue;
	}
	
	// If they form something like:
	// ------
	//     -------
	else if(((MMapEntries[i].Start + MMapEntries[i].Length) > MMapEntries[i + 1].Start) &&
	        ((MMapEntries[i].Start + MMapEntries[i].Length) < 
	         (MMapEntries[i + 1].Start + MMapEntries[i + 1].Length)))
	{
	    if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
	    {
	        // Get the length by subtracting the next end from ours start.
	        MMapEntries[i].Length = (MMapEntries[i + 1].Start + MMapEntries[i + 1].Length) 
		                        - MMapEntries[i].Start;
		// Move the entries up a bit.
		memmove(&MMapEntries[i + 1], &MMapEntries[i + 2], 
			sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 2)));
	        MMapHeader->Entries--;
		
		// We don't know anything about the next entry - so move a entry up, and continue (at same).
		i--;
		continue;
	    }
	    
	    else
	    {
	        // Make space for another entry.
	        memmove(&MMapEntries[i + 2], &MMapEntries[i + 1], 
			sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 1)));
	        MMapHeader->Entries++;                                                       // Increase the number of entries.
	    
	        // The length of the first entry should be equal till where it doesn't overlap.
	        MMapEntries[i].Length = MMapEntries[i + 2].Start - MMapEntries[i].Start;
	    
	        // Take care of the second entry.
	        MMapEntries[i + 1].Start = MMapEntries[i + 2].Start;                         // The second entry starts where the third ends.
	        MMapEntries[i + 1].Length = (MMapEntries[i].Start + MMapEntries[i].Length) 
		                             - MMapEntries[i + 2].Start;
	        MMapEntries[i + 1].Type = Type;                                              // Give it the overrided type.
	        MMapEntries[i + 1].Flags = MMapEntries[i].Flags;                             // Both of the flags should be equal anyway.
	    
	        // And the third entry.
	        MMapEntries[i + 2].Start = MMapEntries[i + 1].Start + MMapEntries[i + 1].Length;       // It start swhere the second ends.
	        MMapEntries[i + 2].Length -= MMapEntries[i + 1].Length;                                // And reduce it's length by whatever the second entry took away.
	    }
	}
	
	// Now, if the look something like:
	// --------
	// ----
	else if((MMapEntries[i].Start == MMapEntries[i + 1].Start) &&
	        (MMapEntries[i].Length > MMapEntries[i + 1].Length))
	{
	    if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
	    {
	        // Move the entries up a bit.
		memmove(&MMapEntries[i + 1], &MMapEntries[i + 2], 
			sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 2)));
	        MMapHeader->Entries--;
		
		// Again, we don't know anything about the next entry. So move up a bit, and continue.
		i--;
		continue;
	    }
	    
	    else
	    {
	        MMapEntries[i + 1].Start = MMapEntries[i + 1].Start + MMapEntries[i + 1].Length;   // Make the second entry start where the previous second ends.
	        MMapEntries[i + 1].Length = (MMapEntries[i].Start + MMapEntries[i].Length) 
		                             - MMapEntries[i + 1].Start;                           // Get the length.
	        MMapEntries[i + 1].Type = MMapEntries[i].Type;                                     // Since this extra portion was from the first entry, get the same type.
	    
	        // And then, fix the first entry.
	        MMapEntries[i].Length -= MMapEntries[i + 1].Length;                                // Remove the extra portion to get the length.
	        MMapEntries[i].Type = Type;                                                        // And the correct type.
	    }
	}
	
	// Something like:
	// -----
	// ---------
	else if((MMapEntries[i].Start == MMapEntries[i + 1].Start) &&
	        (MMapEntries[i + 1].Length > MMapEntries[i].Length))
	{
	    if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
	    {
	        MMapEntries[i].Length = MMapEntries[i + 1].Length;
		// Move the entries up a bit.
		memmove(&MMapEntries[i + 1], &MMapEntries[i + 2], 
			sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 2)));
	        MMapHeader->Entries--;
		
		// Read above.
		i--;
		continue;
	    }
	    
	    else
	    {
	        // Fix the type of the first one - give it the overrided one.
	        MMapEntries[i].Type = Type;
	    
	        MMapEntries[i + 1].Start = MMapEntries[i].Start + MMapEntries[i].Length;
	        MMapEntries[i + 1].Length -= MMapEntries[i].Length;
	    }
	}
	
	// Or something like:
	// -----------
	//    --------
	else if((MMapEntries[i].Start + MMapEntries[i].Length) == 
	        (MMapEntries[i + 1].Start + MMapEntries[i + 1].Length))
	{
	    if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
	    {
	        // Move the entries up a bit.
		memmove(&MMapEntries[i + 1], &MMapEntries[i + 2], 
			sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 2)));
	        MMapHeader->Entries--;
		
		// Here, we deleted the next entry. So, we need to check with it's next entry again. Move back a bit, and continue.
		i--;
		continue;
	    }
	    
	    else
	    {
	        // Fix the length of the first entry.
	        MMapEntries[i].Length = MMapEntries[i + 1].Start - MMapEntries[i].Start;
	    
	        // And then fix the second entry - get the overrided type.
	        MMapEntries[i + 1].Type = Type;
	    }
	}
	
	// And, at last, something like:
	// ---------
	//   -----
	else if((MMapEntries[i + 1].Start + MMapEntries[i + 1].Length) < 
	        (MMapEntries[i].Start + MMapEntries[i].Length))
	{
	    if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
	    {
	        // Move the entries up a bit.
		memmove(&MMapEntries[i + 1], &MMapEntries[i + 2], 
			sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 2)));
	        MMapHeader->Entries--;
		
		// Check with the next' next entry.
		i--;
		continue;
	    }
	  
	    else
	    {
	        // Make space for another entry.
	        memmove(&MMapEntries[i + 3], &MMapEntries[i + 2], 
			sizeof(MMapEntry_t) * (MMapHeader->Entries - i));
	        MMapHeader->Entries++;     
	    
	        MMapEntries[i + 1].Type = Type;      // Give it the overriding type.
	        
	        // And then the third entry.
	        MMapEntries[i + 2].Type = MMapEntries[i].Type;
	        MMapEntries[i + 2].Flags = MMapEntries[i].Flags;
	        MMapEntries[i + 2].Start = MMapEntries[i + 1].Start + MMapEntries[i + 1].Length;
	        MMapEntries[i + 2].Length = (MMapEntries[i].Start + MMapEntries[i].Length) 
		                             - (MMapEntries[i + 1].Start + MMapEntries[i + 1].Length);
	    
	        // Fix the first entry.
	        MMapEntries[i].Length = MMapEntries[i + 1].Start - MMapEntries[i].Start;   
	    }
	}
	
	// And now, check for adjacent ares, with same types.
	if((MMapEntries[i].Type == MMapEntries[i + 1].Type) &&
	   ((MMapEntries[i].Start + MMapEntries[i].Length) == MMapEntries[i + 1].Start))
	{
	    // Increase the length of the first entry.
	    MMapEntries[i].Length += MMapEntries[i + 1].Length;
	
	    // Move the required number of entries from i + 2 to i + 1.
	    memmove(&MMapEntries[i + 1], &MMapEntries[i + 2], 
		    sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 2)));
	    MMapHeader->Entries--; 
	    
	    // Now, we don't know what the next entry is about. So move up a bit.
	    i--;
	    continue;
	}
    }
}

// Initializes the physical memory manager for ourselves.
void PMMInit()
{   
    // Get the addresses into the right variables.
    MMapHeader = (MMapHeader_t*)BIT.MMap;
    MMapEntries = (MMapEntry_t*)MMapHeader->Address;

    PMMFixMMap();                     // Fix overlapping entries. 

    // Allocate space for the BASE_STACK.
    for(uint32_t i = 0; i < MMapHeader->Entries; i++)
    {
        // If we have gone farther than 0x100000, then just break.
        if(MMapEntries[i].Start > 0x100000)
            break;
                    
        // If type isn't Free, then just continue to the next entry.
        if((MMapEntries[i].Type != FREE_RAM))
            continue;
                        
        // Start the base, and keep looping till we are in our entry, or till we are behind the 1-MiB mark.
        for(uint32_t Addr = MMapEntries[i].Start; 
            ((Addr < (MMapEntries[i].Start + MMapEntries[i].Length)) && (Addr < 0x100000));
            Addr += 0x1000)
        { 
            // If this is the first free entry, then make it the Top.
            if(!Top[0])
            {
                Top[0] = (uint32_t*)Addr;
                *Top[0] = 0xAA55AA55;                                                 // 10101010... in binary - marker for end.
            }
            
                                                                    
            else
            {
                // The first dword of each page is the 'next' entry.
                uint32_t Last = (uint32_t)Top[0];
                Top[0] = (uint32_t*)Addr;
                *Top[0] = Last;
            }
        }
    }
    
    // And then, for the upper memory.
    // I could do this in a loop, but I prefer it this way.
    for(uint32_t i = 0; i < MMapHeader->Entries; i++)
    {
        // If we have gone farther than Base + MIN_ALLOC (size), then just break.
        if(MMapEntries[i].Start > (0x100000 + MIN_ALLOC))
            break;
                       
        // If we are below than Base, or type isn't Free, then just continue to the next entry.
        if((MMapEntries[i].Type != FREE_RAM) ||
           (MMapEntries[i].Start < 0x100000))
            continue;
                            
        // Start the base, and keep looping till we are in our entry, or till we are behind the 32-MiB mark.
        for(uint32_t Addr = MMapEntries[i].Start; 
            (Addr < (MMapEntries[i].Start + MMapEntries[i].Length)) && (Addr < (0x100000 + MIN_ALLOC));
            Addr += 0x1000)
        {
            // If we are on a megabyte thingy, and A20 is disabled, skip the megabyte.
            if((BIT.HrdwreFlags & A20_DISABLED) &&
               (Addr & 0x100000))
            {
                // Move to the next megabyte in there.
                Addr = (Addr + 0x100000) & ~0xFFFFF; 
                                                                                
                // And since we are continuing, move one entry behind.
                Addr -= 0x1000;
                continue;
            }
                                                                    
            // If this is the first free entry, then make it the Top.
            if(!Top[1])
            {
                Top[1] = (uint32_t*)Addr;
                *Top[1] = 0xAA55AA55;                                                 // 10101010... in binary - marker for end.
            }
                                                                                                                    
            else
            {
                // The first dword of each page is the 'next' entry.
                uint32_t Last = (uint32_t)Top[1];
                Top[1] = (uint32_t*)Addr;
                *Top[1] = Last;
            }
            
        }
    }
}

// Allocates a frame in the PMM, and returns it's address.
// uint32_t Type                      The type of the frame to allocate - BASE_STACK or POOL_STACK
//     rc
//                                    uint32_t - returns the address of the frame allocated.
//                                    NULL indicates no frame found.
uint32_t PMMAllocFrame(uint32_t Type)
{    
    // If we have reached at the end of the stack:
    if(Top[Type] == (uint32_t*)0xAA55AA55)
        return NULL;
    
    uint32_t *RegionTop = Top[Type];
    uint32_t Frame = (uint32_t)RegionTop;                                                          // Pop the top entry.
    RegionTop = (uint32_t*)RegionTop[0];                                                           // Top[0] contains the last free frame. 
    Top[Type] = RegionTop;
    
    return Frame;
}

// Frees a frame in the PMM.
// uint32_t Type                      The region where to free the frame to - BASE_STACK or POOL_STACK
// uint32_t Addr                      The address of the frame to free.
void PMMFreeFrame(uint32_t Type, uint32_t Addr)
{
    uint32_t *RegionTop = Top[Type];
    
    // The first dword of each page is the 'next' entry.
    uint32_t Last = (uint32_t)RegionTop;
    RegionTop = (uint32_t*)Addr;
    RegionTop[0] = Last;
    
    Top[Type] = RegionTop;
}

// Allocates contiguous number of 'Number' frames.
// uint32_t Type                      The type from where to allocate the frames.
// uint32_t Number                    The number of frames to allocate.
//     rc
//                                    uint32_t - return address of the contiguous frames allocated.
//                                    NULL for can't find.
uint32_t PMMAllocContigFrames(uint32_t Type, uint32_t Number)
{
    uint32_t FramesFound = 1;
    
    uint32_t *RegionTop = Top[Type];
    // Previous is the top, while current is the one after previous.
    uint32_t *Previous = RegionTop;
    uint32_t *Current = (uint32_t*)(*Previous);
    
    // If previous is equal to the magic number, we were unable to find any frame.
    // Abort boot.
    if(Previous == (uint32_t*)0xAA55AA55)
        return NULL;    
    
    while(FramesFound < Number)
    {
        // If current is equal to the magic number, we were unable to find any frame.
        if(Current == (uint32_t*)0xAA55AA55)
	    return NULL;
	
	// If current is equal to the frame previous to 'Previous',
	// then increase the FramesFound count.
	if((uint32_t)Current == ((uint32_t)Previous - 0x1000))
	    FramesFound++;
	
	else
	    FramesFound = 1;          // We've found one page at "Current", which is guaranteed to be a page.
	
	Previous = Current;
	Current = (uint32_t*)(*Previous);
    }
        
    // And, if the contiguous pages found aren't starting from 'Top', make the page at 'top' point to the page at current.
    if((uint32_t)RegionTop != ((uint32_t)Previous + ((Number - 1) * 0x1000)))
        *RegionTop = (uint32_t)Current;
    
    // Else, we start from current.
    else
        RegionTop = Current;
   
    Top[Type] = RegionTop;
     
    return (uint32_t)Previous;
}