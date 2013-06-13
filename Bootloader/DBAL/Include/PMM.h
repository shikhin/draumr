/* 
 * Contains common PMM definitions.
 *
 * Copyright (c) 2013, Shikhin Sethi
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

#ifndef _PMM_H
#define _PMM_H

#include <Standard.h>

// The Header structure for the MMap.
struct MMapHeader
{
    uint16_t Entries;                // The number of entries in the memory map.
    uint32_t Address;                 // The starting address of the memory map.
}_PACKED;

// The Entry structure for the MMap.
struct MMapEntry
{
    // The Start of the entry, and it's length.
    uint64_t Start;
    uint64_t Length;

    // The Type of the entry, and it's Length.
    uint32_t Type;
    uint32_t Flags;
}_PACKED;

typedef struct MMapHeader MMapHeader_t;
typedef struct MMapEntry MMapEntry_t;

/* The RAM Types - in case I forget before I write the docs :P */
// Type 1 - Free RAM
// Type 2 - Boot Code
// Type 3 - ACPI Reclaimable
// Type 4 - Unusable RAM
// Type 5 - ACPI NVS
// Type 6 - Bad RAM
// The USABLE flag - is the first bit.
#define USABLE    (1 << 0)

// The types - some macros to make it easy and more beautiful :-)
#define FREE_RAM  1

#define BASE_BITMAP                    0
#define POOL_BITMAP                    1

// The "remove entry" macro.
#define REMOVE_ENTRY(EntryNumber) {                                                                               \
                                    memmove(&MMapEntries[EntryNumber], &MMapEntries[EntryNumber + 1],            \
                                            sizeof(MMapEntry_t) * (MMapHeader->Entries - (EntryNumber + 1)));    \
                                    MMapHeader->Entries--;                                                         \
                                  }

// Define the pointers to the headers and entries.
extern MMapHeader_t *MMapHeader;
extern MMapEntry_t *MMapEntries;

/*
 * Initializes the physical memory manager for ourselves.
 */
void PMMInit(void) _COLD;

/*
 * Allocates a frame in the PMM, and returns it's address.
 *     uint32_t Type -> the type of the frame to allocate - BASE_BITMAP or POOL_BITMAP
 *
 * Returns:
 *     uint32_t      -> the address of the frame allocated. 0 indicates error.
 */
uint32_t PMMAllocFrame(uint32_t Type);

/*
 * Frees a frame in the PMM.
 *     uint32_t Addr -> the address of the frame to free.
 */
void PMMFreeFrame(uint32_t Addr);

/*
 * Allocates contiguous number of 'Number' frames.
 *     uint32_t Type   -> the type from where to allocate the frames.
 *     uint32_t Number -> the number of frames to allocate.
 *
 * Returns:
 *     uint32_t        -> the address of the contiguous frames allocated.
 */
uint32_t PMMAllocContigFrames(uint32_t Type, uint32_t Number);

/*
 * Frees contiguous number of 'Number' frames.
 *     uint32_t Addr   -> the starting address from where to free.
 *     uint32_t Number -> the number of frames to free.
 */
void PMMFreeContigFrames(uint32_t Addr, uint32_t Number);

#endif                                /* PMM.h */
