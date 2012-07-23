/*
 * Contains VMM (PAE) related functions for KL.
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

// Define VMMPAE so that the related things are defined in VMM.h
#define VMMPAE_PAGING

#include <VMM.h>
#include <BIT.h>

// Define pointers for the page directory pointer table - aligned by 32.
PageDirPTEntry_t PDPT[4] _ALIGNED(0x20);

/*
 * Initializes PAE paging.
 */
void PAEPagingInit()
{
    // Allocate a page for the page directory.
    PDPT[PDPT_INDEX(0x00000000)] = (PageDirPTEntry_t)(BIT->DBALPMM.AllocFrame(POOL_BITMAP) | PRESENT_BIT);

    // Allocate a page for the page table for identity mapping the 1st MiB.
    PageTableEntry_t *BaseTable = (PageTableEntry_t*)BIT->DBALPMM.AllocFrame(POOL_BITMAP);
    PageDirEntry_t   *PageDir   = (PageDirEntry_t*)(uint32_t)(PDPT[PDPT_INDEX(0x00000000)] & PAGE_MASK);

    PageDir[PD_INDEX(0x00000000)] = (PageDirEntry_t)BaseTable | PRESENT_BIT;

    for(uint32_t Index = 0x0000; Index < 0x100000; Index += 0x1000)
    {
        BaseTable[PT_INDEX(Index)] = Index | PRESENT_BIT;
    }
}

/*
 * Maps a page (PAE).
 *     uint64_t VirtAddr -> the virtual address where to map the frame to.
 *     uint64_t PhysAddr -> the physical address of the frame to map to the page.
 */
void PAEPagingMap(uint64_t VirtAddr, uint64_t PhysAddr)
{
    PageDirEntry_t *PageDir; PageTableEntry_t *PageTable;
    // If page directory isn't present, make one.
    if(!(PDPT[PDPT_INDEX(VirtAddr)] & PRESENT_BIT))
    {
        PageDir = (PageDirEntry_t*)BIT->DBALPMM.AllocFrame(POOL_BITMAP);
        PDPT[PDPT_INDEX(VirtAddr)] = (PageDirPTEntry_t)PageDir | PRESENT_BIT;
    }

    else
    {
        PageDir = (PageDirEntry_t*)(uint32_t)(PDPT[PDPT_INDEX(VirtAddr)] & PAGE_MASK);
    }

    // If page table isn't present, make one.
    if(!(PageDir[PD_INDEX(VirtAddr)] & PRESENT_BIT))
    {
        PageTable = (PageTableEntry_t*)BIT->DBALPMM.AllocFrame(POOL_BITMAP);
        PageDir[PD_INDEX(VirtAddr)] = (PageDirEntry_t)PageTable | PRESENT_BIT;
    }

    else
    {
        PageTable = (PageTableEntry_t*)(uint32_t)(PageDir[PD_INDEX(VirtAddr)] & PAGE_MASK);
    }

    PageTable[PT_INDEX(VirtAddr)] = PhysAddr | PRESENT_BIT;
}

// Un-define VMMPAE_PAGING.
#undef VMMPAE_PAGING
