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

#ifndef PMM_H                         /* PMM.h */
#define PMM_H

#include <stdint.h>

// The Header structure for the MMap.
struct MMapHeader
{
    uint16_t Entries;                 // The number of entries in the memory map.
    uint32_t Address;                 // The starting address of the memory map.
} __attribute__((packed));

// The Entry structure for the MMap.
struct MMapEntry
{
    // The Start of the entry, and it's length.
    uint64_t Start;
    uint64_t Length;
    
    // The Type of the entry, and it's Length.
    uint32_t Type;
    uint32_t Flags;
} __attribute__((packed));

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

// We'll be checking till here only.
#define BASE      0x2000000

// The types - some macros to make it easy and more beautiful :-)
#define FREE_RAM  1

// Define the pointers to the headers and entries.
extern MMapHeader_t *MMapHeader;
extern MMapEntry_t  *MMapEntries;

// Initializes the physical memory manager for ourselves.
void PMMInit();

#endif                                /* PMM.h */