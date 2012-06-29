/* 
 * Contains common PMM definitions.
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
 *     * Neither the name of Draumr nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <PMM.h>
#include <String.h>
#include <BIT.h>
#include <Abort.h>
#include <Bitmap.h>

// Define the pointers to the headers and entries.
MMapHeader_t *MMapHeader;
MMapEntry_t *MMapEntries;

// Define the bitmap for the 'pool' and 'base' region.
static Bitmap_t BaseBitmap;
static Bitmap_t PoolBitmap;

/*
 * Fixes the PMM memory map - including the overlapping regions, merging consecutive regions, and others.
 */
static void PMMFixMMap()
{
    // Fix all entries in all possible ways here, making a clean sweep!
    for(uint16_t i = 0; i < (MMapHeader->Entries - 1); i++)
    {
        // If the flags field isn't equal, then just continue - the flags can virtually make different entries.
        if(MMapEntries[i].Flags != MMapEntries[i + 1].Flags)
            continue;

        // Get the recommended type, the maximum of the both. Since types are overriding, this should result in the "best mode".
        uint32_t Type = MAX(MMapEntries[i].Type, MMapEntries[i + 1].Type);

        // If the entries are the same start and length:
        // ******
        // ******
        if((MMapEntries[i].Start == MMapEntries[i + 1].Start)
                && (MMapEntries[i].Length == MMapEntries[i + 1].Length))
        {
            MMapEntries[i].Type = Type;
            REMOVE_ENTRY((i + 1))

            // Here, we don't know whether we overlap with the next entry or not. So, move to previous, and continue.
            i--;
            continue;
        }

        // If the entries overlap in the standard way:
        // ******
        //    ******
        else if(( (MMapEntries[i].Start + MMapEntries[i].Length)
                > MMapEntries[i + 1].Start)
                && ((MMapEntries[i].Start + MMapEntries[i].Length)
                        < (MMapEntries[i + 1].Start + MMapEntries[i + 1].Length)))
        {
            // If type's are same, then remove one entry, and increase the length in one entry.
            if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
            {
                // Get the length by subtracting the next end from ours start.
                MMapEntries[i].Length = (MMapEntries[i + 1].Start
                        + MMapEntries[i + 1].Length) - MMapEntries[i].Start;

                // Move the entries up a bit.
                REMOVE_ENTRY((i + 1))

                // We don't know anything about the next entry - so move a entry up, and continue (at same).
                i--;
                continue;
            }

            // Else, make space for another entry for the overlapping region.
            else
            {
                // Make space for another entry.
                memmove(&MMapEntries[i + 2], &MMapEntries[i + 1],
                        sizeof(MMapEntry_t) * (MMapHeader->Entries - (i + 1)));

                // Increase the number of entries. 
                MMapHeader->Entries++;

                // Take care of the second entry.
                // The second entry starts where the earlier second entry started.
                MMapEntries[i + 1].Start = MMapEntries[i + 2].Start;
                MMapEntries[i + 1].Length = (MMapEntries[i].Start
                        + MMapEntries[i].Length) - MMapEntries[i + 2].Start;

                // Give it the overrided type.
                MMapEntries[i + 1].Type = Type;
                MMapEntries[i + 1].Flags = MMapEntries[i].Flags;

                // The first entry.
                // The length of the first entry should be equal till where it doesn't overlap.
                MMapEntries[i].Length = MMapEntries[i + 2].Start
                        - MMapEntries[i].Start;

                // And the third entry.
                // It starts at where the second entry ends, and reduce it's length by length of second entry.
                MMapEntries[i + 2].Start = MMapEntries[i + 1].Start
                        + MMapEntries[i + 1].Length;
                MMapEntries[i + 2].Length -= MMapEntries[i + 1].Length;
            }
        }

        // If the starting of the entries were same, while the second entry was shorter than the first one:
        // ******
        // ***
        else if((MMapEntries[i].Start == MMapEntries[i + 1].Start)
                && (MMapEntries[i].Length > MMapEntries[i + 1].Length))
        {
            // If the types are same, remove the second (smaller) entry.
            if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
            {
                // Move the entries up a bit.
                REMOVE_ENTRY((i + 1))

                // Again, we don't know anything about the next entry. So move up a bit, and continue.
                i--;
                continue;
            }

            // Else, make the second entry the entry for the "non-overlapping" region.
            else
            {
                // Make the second entry start where the previous second ends (the non-overlapping region).
                MMapEntries[i + 1].Start = MMapEntries[i + 1].Start
                        + MMapEntries[i + 1].Length;

                // And make it's length the length of the non-overlapping region.
                MMapEntries[i + 1].Length = (MMapEntries[i].Start
                        + MMapEntries[i].Length) - MMapEntries[i + 1].Start;

                // And let it's type remain the original type of the non overlapping region.
                MMapEntries[i + 1].Type = MMapEntries[i].Type;

                // Fix the first entry.

                // Get it's new length.
                MMapEntries[i].Length -= MMapEntries[i + 1].Length;
                // And set it's type as the overriding type.
                MMapEntries[i].Type = Type;
            }
        }

        // If the entries begin at the same address, and the first one is smaller than the second one:
        // ***
        // ******
        else if((MMapEntries[i].Start == MMapEntries[i + 1].Start)
                && (MMapEntries[i + 1].Length > MMapEntries[i].Length))
        {
            // Is both are of same type, delete the first one.
            if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
            {
                // Give the first one the properties of the second one.
                MMapEntries[i].Length = MMapEntries[i + 1].Length;

                // And delete the second one.
                REMOVE_ENTRY((i + 1))

                // Repeat the same entry to test again.
                i--;
                continue;
            }

            // Else, edit the second entry to cover the non-overlapping region.
            else
            {
                // Give the first entry the overriding type.
                MMapEntries[i].Type = Type;

                // Give the second entry the starting address and length of the non-overlapping region.
                MMapEntries[i + 1].Start = MMapEntries[i].Start
                        + MMapEntries[i].Length;
                MMapEntries[i + 1].Length -= MMapEntries[i].Length;
            }
        }

        // If length of both entries adds up to them ending at the same address:
        // ******
        //    ***
        else if((MMapEntries[i].Start + MMapEntries[i].Length)
                == (MMapEntries[i + 1].Start + MMapEntries[i + 1].Length))
        {
            // If types of both the entries are same, then delete the second entry.
            if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
            {
                // Move the entries up a bit.
                REMOVE_ENTRY((i + 1))

                // Here, we deleted the next entry. So, we need to check with it's next entry again. Move back a bit, and continue.
                i--;
                continue;
            }

            // Else, reduce the length of the first entry, and change the type of the second entry.
            else
            {
                // Fix the length of the first entry.
                MMapEntries[i].Length = MMapEntries[i + 1].Start
                        - MMapEntries[i].Start;

                // And then fix the type of the second entry..
                MMapEntries[i + 1].Type = Type;
            }
        }

        // If second entry is 'inside' first entry, without touching the borders:
        // ******
        //  ****
        else if((MMapEntries[i + 1].Start + MMapEntries[i + 1].Length)
                < (MMapEntries[i].Start + MMapEntries[i].Length))
        {
            // If types are same, remove second entry.
            if(MMapEntries[i].Type == MMapEntries[i + 1].Type)
            {
                // Move the entries up a bit.
                REMOVE_ENTRY((i + 1))

                // Check with the next' next entry.
                i--;
                continue;
            }

            // Else, divide it into three: 'starting', 'middle (overlapping)' and 'ending'.
            else
            {
                // Make space for another entry.
                memmove(&MMapEntries[i + 3], &MMapEntries[i + 2],
                        sizeof(MMapEntry_t) * (MMapHeader->Entries - i));
                MMapHeader->Entries++;

                // Give the entry containing just the overlapping region the overrided type.
                MMapEntries[i + 1].Type = Type;

                // Fix the third entry.
                MMapEntries[i + 2].Type = MMapEntries[i].Type;
                MMapEntries[i + 2].Flags = MMapEntries[i].Flags;

                // Make its start the end of the overlapping region, and it's length the length of the 'end' region.
                MMapEntries[i + 2].Start = MMapEntries[i + 1].Start
                        + MMapEntries[i + 1].Length;
                MMapEntries[i + 2].Length =
                        (MMapEntries[i].Start + MMapEntries[i].Length)
                                - (MMapEntries[i + 1].Start
                                        + MMapEntries[i + 1].Length);

                // Fix the first entry.
                MMapEntries[i].Length = MMapEntries[i + 1].Start
                        - MMapEntries[i].Start;
            }
        }

        // Check whether it is adjacent areas of same type, and if yes, merge.
        if((MMapEntries[i].Type == MMapEntries[i + 1].Type)
                && ((MMapEntries[i].Start + MMapEntries[i].Length)
                        == MMapEntries[i + 1].Start))
        {
            // Increase the length of the first entry.
            MMapEntries[i].Length += MMapEntries[i + 1].Length;

            // Move the required number of entries from i + 2 to i + 1.
            REMOVE_ENTRY((i + 1))

            // Now, we don't know what the next entry is about. So move up a bit.
            i--;
            continue;
        }
    }

    // Fix all entries not starting on page boundaries.
    for(uint16_t i = 0; i < MMapHeader->Entries; i++)
    {
        // Align the start address to the highest page boundary, while the length to the lowest.
        MMapEntries[i].Start = (MMapEntries[i].Start + 0xFFF) & ~0xFFF;
        MMapEntries[i].Length &= ~0xFFF;

        // If the length is zero, then remove the entry.
        if(!MMapEntries[i].Length)
        {
            // Move the required number of entries from i + 1 to i.
            REMOVE_ENTRY((i))

            // Here, we don't know about the current entry, so, move to previous, and continue.
            i--;
            continue;
        }
    }
}

/*
 * Initialize the bitmaps.
 */
static void InitializeBitmap()
{
    for(uint16_t i = 0; i < MMapHeader->Entries; i++)
    {
        // If it isn't free RAM, loop.
        if(MMapEntries[i].Type != FREE_RAM)
        {
            continue;
        }

        // Loop over each entry.
        for(uint64_t Addr = MMapEntries[i].Start;
                Addr < (MMapEntries[i].Start + MMapEntries[i].Length); Addr +=
                        0x1000)
        {
            // If address is smaller than 1MiB, initialize it in the base bitmap.
            if(Addr < 0x100000)
            {
                BaseBitmap.Data[INDEX_BIT(Addr / 0x1000)] &= ~ (1
                        << OFFSET_BIT(Addr / 0x1000));
            }

            // Else, in the pool bitmap.
            else
            {
                PoolBitmap.Data[INDEX_BIT((Addr - 0x100000) / 0x1000)] &= ~ (1
                        << OFFSET_BIT((Addr - 0x100000) / 0x1000));
            }
        }
    }
}

/*
 * Initializes the physical memory manager for ourselves.
 */
void PMMInit()
{
    // Get the addresses into the right variables.
    MMapHeader = (MMapHeader_t*)BIT.MMap;
    MMapEntries = (MMapEntry_t*)MMapHeader->Address;

    // Fix overlapping entries.
    PMMFixMMap();

    // Find the highest address (last region) described by the pool bitmap.
    uint64_t HighestAddress = (MMapEntries[MMapHeader->Entries - 1].Length
            + MMapEntries[MMapHeader->Entries - 1].Start);

    // Calculate the size of the bitmap.
    uint32_t BitmapSize = (((HighestAddress / 0x1000) + 31) & ~31) / 8;
    BitmapSize = (BitmapSize + 0xFFF) & ~0xFFF;

    // Try to find a region in the memory map to hold that much space. 
    for(uint16_t i = 0; i < MMapHeader->Entries; i++)
    {
        // If it isn't free RAM, then continue.
        if(MMapEntries[i].Type != FREE_RAM)
        {
            continue;
        }

        // If the starting address is over 0xFFFF0000, then simply break..
        else if((MMapEntries[i].Start + MMapEntries[i].Length) > 0xFFFF0000)
        {
            break;
        }

        // If, the entry can accomodate the bitmap, and, the address doesn't go over the 4GiB boundary, then use that area.
        else if((MMapEntries[i].Length >= BitmapSize)
                && ((MMapEntries[i].Start + BitmapSize) <= 0xFFFF0000))
        {
            // Asign the 'data' and 'size' of the pool and base bitmap.
            BaseBitmap.Data = (uint32_t*) ((uint32_t)MMapEntries[i].Start
                    + (uint32_t)MMapEntries[i].Length - (uint32_t)BitmapSize);
            BaseBitmap.Size = (0x100000 / 0x1000);

            PoolBitmap.Data = BaseBitmap.Data + (BaseBitmap.Size / 32);
            PoolBitmap.Size = (HighestAddress - 0x100000) / 0x1000;

            // Decrease the length, accordingly.
            MMapEntries[i].Length -= BitmapSize;

            // If length is zero, remove the entry.
            if(!MMapEntries[i].Length)
            {
                // Move the required number of entries from i + 1 to i.
                REMOVE_ENTRY((i))

                // Here, we don't know about the current entry, so, move to previous, and continue.
                i--;
            }

            break;
        }
    }

    // If the a suitable entry wasn't found, abort with error.
    if(!BaseBitmap.Size)
    {
        AbortBoot("Unable to allocate enough space for boot bitmaps.\n");
    }

    // Initialize the bitmaps (to 0xFFFFFFFF).
    BaseBitmap = BitmapInit(BaseBitmap.Data, BaseBitmap.Size, 0xFFFFFFFF);
    PoolBitmap = BitmapInit(PoolBitmap.Data, PoolBitmap.Size, 0xFFFFFFFF);

    // Initialize the bitmaps, now.
    InitializeBitmap();

    // Update the instance of the first zero bit in both the bitmaps.
    BaseBitmap.FirstZero = FindFirstZero(&BaseBitmap, BaseBitmap.FirstZero);
    PoolBitmap.FirstZero = FindFirstZero(&PoolBitmap, PoolBitmap.FirstZero);
}

/*
 * Allocates a frame in the PMM, and returns it's address.
 *     uint32_t Type -> the type of the frame to allocate - BASE_BITMAP or POOL_BITMAP
 *
 * Returns:
 *     uint32_t      -> the address of the frame allocated. 0 indicates error.
 */
uint32_t PMMAllocFrame(uint32_t Type)
{
    Bitmap_t *Bitmap;
    uint32_t Frame;
    int64_t Bit;

    // If it's the base bitmap, then point to the base bitmap.
    if(Type == BASE_BITMAP)
    {
        Bitmap = &BaseBitmap;
    }

    // Else, the pool bitmap.
    else
    {
        Bitmap = &PoolBitmap;
    }

    // Get the bit returned by the bitmap.
    Bit = BitmapFindFirstZero(Bitmap);
    if(Bit == -1)
    {
        return (uint32_t)NULL;
    }

    if(((uint64_t)Bit * 0x1000ULL) > 0xFFFF0000ULL)
    {
        BitmapClearBit(Bitmap, Bit);
        return (uint32_t)NULL;
    }

    Frame = Bit * 0x1000;
    if(Type == POOL_BITMAP)
    {
        // The pool bitmap internally starts from 1MiB above - so increase the Frame's address.
        Frame += 0x100000;
    }

    return Frame;
}

/*
 * Frees a frame in the PMM.
 *     uint32_t Addr -> the address of the frame to free.
 */
void PMMFreeFrame(uint32_t Addr)
{
    Bitmap_t *Bitmap;

    // If it's the base bitmap, then point to the base bitmap.
    if(Addr < 0x100000)
    {
        Bitmap = &BaseBitmap;
    }

    // Else, point to the pool bitmap.
    else
    {
        Bitmap = &PoolBitmap;

        // And since the pool bitmap internally starts from 1MiB above, decrease the frame's address.
        Addr -= 0x100000;
    }

    BitmapClearBit(Bitmap, Addr / 0x1000);
}

/*
 * Allocates contiguous number of 'Number' frames.
 *     uint32_t Type   -> the type from where to allocate the frames.
 *     uint32_t Number -> the number of frames to allocate.
 *
 * Returns:
 *     uint32_t        -> the address of the contiguous frames allocated.
 */
uint32_t PMMAllocContigFrames(uint32_t Type, uint32_t Number)
{
    Bitmap_t *Bitmap;
    uint32_t Frame;
    int64_t Bit;

    // If it's the base bitmap, then point to the base bitmap.
    if(Type == BASE_BITMAP)
    {
        Bitmap = &BaseBitmap;
    }

    // Else, the pool bitmap.
    else
    {
        Bitmap = &PoolBitmap;
    }

    // Get the bit returned by the bitmap.
    Bit = BitmapFindContigZero(Bitmap, Number);

    // If was unable to allocate the bit, or it spans above 4GiB.
    if(Bit == -1)
    {
        return (uint32_t)NULL;
    }

    Bit += Number;
    if(((uint64_t)Bit * 0x1000ULL) > 0xFFFF0000ULL)
    {
        BitmapClearContigZero(Bitmap, Bit - Number, Number);
        return (uint32_t)NULL;
    }

    Bit -= Number;
    Frame = Bit * 0x1000;

    if(Type == POOL_BITMAP)
    {
        // The pool bitmap internally starts from 1MiB above - so increase the Frame's address.
        Frame += 0x100000;
    }

    return Frame;
}

/*
 * Frees contiguous number of 'Number' frames.
 *     uint32_t Addr   -> the starting address from where to free.
 *     uint32_t Number -> the number of frames to free.
 */
void PMMFreeContigFrames(uint32_t Addr, uint32_t Number)
{
    Bitmap_t *Bitmap;

    // If it's the base bitmap, then point to the base bitmap.
    if(Addr < 0x100000)
    {
        Bitmap = &BaseBitmap;
    }

    // Else, point to the pool bitmap.
    else
    {
        Bitmap = &PoolBitmap;

        // And since the pool bitmap internally starts from 1MiB above, decrease the frame's address.
        Addr -= 0x100000;
    }

    BitmapClearContigZero(Bitmap, Addr / 0x1000, Number);
}
