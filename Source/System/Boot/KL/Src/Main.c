/*
 * Entry point for KL.
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

#include <Standard.h>
#include <BIT.h>
#include <CPU.h>
#include <VMM.h>

// A pointer to the BIT structure.
BIT_t *BIT;

/*
 * Maps a page (generic).
 *     uint64_t VirtAddr -> the virtual address where to map the frame to.
 *     uint64_t PhysAddr -> the physical address of the frame to map to the page.
 */
_PROTOTYPE(void (*GenericPagingMap), (uint64_t VirtAddr, uint64_t PhysAddr));

/*
 * Maps a file (following the generic file header) to the virtual address.
 *     FILE_t FILE -> the file structure.
 */
void ModuleMap(FILE_t FILE)
{
    uint64_t *Location, StartAddr, EndAddr;

    // Calculate the location & start and ending virtual address.
    Location = (uint64_t*)FILE.Location;
    StartAddr = *(uint64_t*)((uint8_t*)Location + 12);
    EndAddr = (*(uint64_t*)((uint8_t*)Location + 20) + 0xFFF) & PAGE_MASK;

    for(; StartAddr < EndAddr; StartAddr += 0x1000, Location = (uint64_t*)((uint8_t*)Location + 0x1000))
    {
        GenericPageMap(StartAddr, (uint64_t)Location);
    }
}

/* 
 * The Main function for the KL sub-module.
 *     uint32_t *BITPointer -> the pointer to the BIT.
 */
void Main(BIT_t *BITPointer)
{
    // Save BITPointer into a global variable.
    BIT = BITPointer;

    uint32_t FeatureFlags = CPUFeatureFlags();
    FILE_t Kernel, KernelMPMM, KernelMVMM;

    // Long mode is present - load the related files.
    if(FeatureFlags & LONG_MODE_PRESENT)
    {
        // Load the AMD64 kernel.
        BIT->FileAPI(FILE_KERNEL, ARCH_AMD64, &Kernel);

        // Load the PMM and VMM AMD64 kernel module.
        BIT->FileAPI(FILE_KERNEL_M, PMMAMD64, &KernelMPMM);
        BIT->FileAPI(FILE_KERNEL_M, VMMAMD64, &KernelMVMM);

        AMD64PagingInit();

        // Point the paging map function to AMD64 one.
        GenericPagingMap = &AMD64PagingMap;

        // Set the architecture.
        BIT->Arch = ARCH_AMD64;
    }

    // Else, load the x86 files.
    else
    {
        // Load the x86 kernel.
        BIT->FileAPI(FILE_KERNEL, ARCH_X86, &Kernel);

        // If PAE is present, then load those modules.
        // ALSO, NOTE: Memory *should* be present over 4GiB to take advantage of PAE, so we
        // ensure there is. Else, we use x86.
        if((FeatureFlags & PAE_PRESENT) &&
           (BIT->HighestAddress > 0xFFFFFFFFLLU))
        {
            BIT->FileAPI(FILE_KERNEL_M, PMMX86PAE, &KernelMPMM);
            BIT->FileAPI(FILE_KERNEL_M, VMMX86PAE, &KernelMVMM);

            PAEPagingInit();

            // Point the paging map function to PAE one.
            GenericPagingMap = &PAEPagingMap;

            // Set the architecture.
            BIT->Arch = ARCH_PAE;
        }

        // Else, load the x86 modules.
        else
        {
            BIT->FileAPI(FILE_KERNEL_M, PMMX86, &KernelMPMM);
            BIT->FileAPI(FILE_KERNEL_M, VMMX86, &KernelMVMM);

            x86PagingInit();

            // Point the paging map function to x86 one.
            GenericPagingMap = &x86PagingMap;

            // Set the architecture.
            BIT->Arch = ARCH_X86;
        }
    }

    // Define some variables for mapping the kernel, pmm module and vmm module.
    uint64_t *Location, StartAddr, EndAddr;

    // Map the kernel (& modules).
    ModuleMap(Kernel);
    ModuleMap(KernelMPMM);
    ModuleMap(KernelMVMM);

    // We shouldn't reach here.
    for(;;)
        __asm__ __volatile__("hlt");
}
