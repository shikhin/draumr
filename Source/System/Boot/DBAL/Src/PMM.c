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

// Define the pointers to the headers and entries.
MMapHeader_t *MMapHeader;
MMapEntry_t  *MMapEntries;

// The MAX macro, move it to it's own private place sometime.
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// The Top of the PMM stack.
static uint32_t *Top;
static uint32_t Base = 0x00000000;               // Allocate extra memory starting from here in the mmap.

// PMMFixMMap fixes the memory map - only overlapping entries.
static void PMMFixMMap()
{
    // Loop till all the entries.
    for(uint16_t i = 0; i < (MMapHeader->Entries - 1); i++)
    {
        // And the length and the start address to the below page boundary.
        MMapEntries[i].Start &= ~0xFFF;
	MMapEntries[i].Length = (MMapEntries[i].Length + 0xFFF) & ~0xFFF;
	
	MMapEntries[i + 1].Start &= ~0xFFF;
	MMapEntries[i + 1].Length = (MMapEntries[i + 1].Length + 0xFFF) & ~0xFFF;
	
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

// Allocates some extra memory from the memory map, in the provided region.
//     rc
//                                    uint32_t - the number of frames allocated.
static uint32_t PMMGetSpace()
{
    uint32_t Allocated = 0;
    
    // If we reached 0xFE000000, and still need more memory, then some thing is WRONG.
    if(Base == 0xFE000000)
    {
        // TODO: Beeping here.
        DebugPrintText("ERROR: The Boot Abstraction Layer wants too much memory.\n");
	for(;;)
	    __asm__ __volatile__("hlt");
    }
 
    for(uint32_t i = 0; i < MMapHeader->Entries; i++)
    {
        // If we have gone farther than Base + BASE (size), then just break.
        if(MMapEntries[i].Start > (Base + BASE))
	    break;
	
	// If we are below than Base, or type isn't Free, then just continue to the next entry.
	if((MMapEntries[i].Type != FREE_RAM) ||
	   (MMapEntries[i].Start < Base))
	    continue;
	    
	// Start the base, and keep looping till we are in our entry, or till we are behind the 32-MiB mark.
        for(uint32_t Addr = MMapEntries[i].Start; 
	    (Addr < (MMapEntries[i].Start + MMapEntries[i].Length)) && (Addr < (Base + BASE));
	    Addr += 0x1000)
	{
	    // If this is the first free entry, then make it the Top.
	    if(!Top)
	    {
		Top = (uint32_t*)Addr;
		Top[0] = 0xAA55AA55;                                                     // 10101010... in binary - marker for end.
		Allocated++;
	    }
		
	    else
	    {
	        // The first dword of each page is the 'next' entry.
	        uint32_t Last = (uint32_t)Top;
	        Top = (uint32_t*)Addr;
		Top[0] = Last;
		Allocated++;                                                             // Increase the frames allocated count.
	    }
	}
    }
    
    // Increase the base counter, and return with Allocated.
    Base += BASE;
    return Allocated;
}

// Initializes the physical memory manager for ourselves.
void PMMInit()
{    
    // Get the addresses into the right variables.
    MMapHeader = (MMapHeader_t*)BIT.MMap;
    MMapEntries = (MMapEntry_t*)MMapHeader->Address;

    PMMFixMMap();                     // Fix overlapping entries. 
    
    // Keep looping till PMMGetSpace returns something. 
    while(!(PMMGetSpace()));
}

// Allocates a frame in the PMM, and returns it's address.
//     rc
//                                    uint32_t - returns the address of the frame allocated.
uint32_t PMMAllocFrame()
{
    // If we have reached at the end of the stack:
    if(Top == (uint32_t*)0xAA55AA55)
    {
        Top = (uint32_t*)0x00000000;                                                               // Clear out Top.
        while(!(PMMGetSpace()));                                                                   // And keep looping till we get somehing
    }
    
    uint32_t Frame = (uint32_t)Top;                                                                // Pop the top entry.
    Top = (uint32_t*)Top[0];                                                                       // Top[0] contains the last free frame. 
    
    return Frame;
}

// Frees a frame in the PMM.
// uint32_t Addr                      The address of the frame to free.
void PMMFreeFrame(uint32_t Addr)
{
    // The first dword of each page is the 'next' entry.
    uint32_t Last = (uint32_t)Top;
    Top = (uint32_t*)Addr;
    Top[0] = Last;
}