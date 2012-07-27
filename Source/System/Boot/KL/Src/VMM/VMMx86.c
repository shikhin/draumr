/*
 * Contains VMM (x86) related functions for KL.
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

// Define VMMX86 so that the related things are defined in VMM.h
#define VMMX86_PAGING

#include <VMM.h>
#include <BIT.h>
#include <String.h>

// Define pointers for the page directory.
PageDirEntry_t *PageDir;

/*
 * Initializes x86 paging.
 */
void x86PagingInit()
{
    // Allocate a page for the page directory, and clear it..
    PageDir = (PageDirEntry_t*)BIT->DBALPMM.AllocFrame(POOL_BITMAP);
    memset(PageDir, 0x00000000, PAGE_SIZE);

    // Allocate a page table for identity mapping the 1st MiB, and clear it.
    PageTableEntry_t *BaseTable = (PageTableEntry_t*)BIT->DBALPMM.AllocFrame(POOL_BITMAP);
    memset(BaseTable, 0x00000000, PAGE_SIZE);

    PageDir[PD_INDEX(0x00000000)] = (PageDirEntry_t)BaseTable | PRESENT_BIT;

    for(uint32_t Index = 0x0000; Index < 0x100000; Index += 0x1000)
    {
        BaseTable[PT_INDEX(Index)] = Index | PRESENT_BIT;
    }

    // Self-recursive trick, ftw!
    PageDir[1023] = (PageDirEntry_t)PageDir | PRESENT_BIT;
}

/*
 * Maps a page (x86).
 *     uint64_t VirtAddr -> the virtual address where to map the frame to.
 *     uint64_t PhysAddr -> the physical address of the frame to map to the page.
 */
void x86PagingMap(uint64_t VirtAddr, uint64_t PhysAddr)
{
    PageTableEntry_t *PageTable;
    // If page table isn't present, make one.
    if(!(PageDir[PD_INDEX(VirtAddr)] & PRESENT_BIT))
    {
        PageTable = (PageTableEntry_t*)BIT->DBALPMM.AllocFrame(POOL_BITMAP);
        memset(PageTable, 0x00000000, PAGE_SIZE);

        PageDir[PD_INDEX(VirtAddr)] = (PageDirEntry_t)PageTable | PRESENT_BIT;
    }

    else
    {
        PageTable = (PageTableEntry_t*)(PageDir[PD_INDEX(VirtAddr)] & PAGE_MASK);
    }

    PageTable[PT_INDEX(VirtAddr)] = (PageTableEntry_t)PhysAddr | PRESENT_BIT;
}

/*
 * Enables x86 paging, and jumps to kernel.
 */
void x86PagingEnable()
{
    // Put the address of page directory in CR3.
    __asm__ __volatile__("mov %0, %%cr3" :: "r"(PageDir));

    // Enable paging (PG bit).
    __asm__ __volatile__("mov %%cr0, %%eax;"
                         "or  $0x80000000, %%eax;"
                         "mov %%eax, %%cr0" ::: "eax");

    // Jump to the kernel.
    __asm__ __volatile__("jmp *0xC0000004");

    // We shouldn't reach here.
    for(;;)
    {
        __asm__ __volatile__("hlt");
    }
}

// Un-define VMMX86_PAGING.
#undef VMMX86_PAGING
