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

BIT_t *BIT;

/* 
 * The Main function for the KL sub-module.
 *     uint32_t *BITPointer -> the pointer to the BIT.
 */
void Main(BIT_t *BITPointer)
{
    // Save BITPointer into a global variable.
    BIT = BITPointer;

    uint32_t FeatureFlags = CPUFeatureFlags();

    // Long mode is present - load the related files.
    if(FeatureFlags & LONG_MODE_PRESENT)
    {
        // Load the AMD64 kernel.
        FILE_t KernelAMD64File;
        BIT->FileAPI(FILE_KERNEL, ARCH_AMD64, &KernelAMD64File);

        // Load the PMM and VMM AMD64 kernel module.
        FILE_t KernelMPMM, KernelMVMM;
        BIT->FileAPI(FILE_KERNEL_M, PMMAMD64, &KernelMPMM);
        BIT->FileAPI(FILE_KERNEL_M, VMMAMD64, &KernelMVMM);
    }

    // Else, load the x86 files.
    else
    {
        // Load the x86 kernel.
        FILE_t Kernelx86File;
        BIT->FileAPI(FILE_KERNEL, ARCH_X86, &Kernelx86File);

        // Load the needed PMM and VMM kernel modules.
        FILE_t KernelMPMM, KernelMVMM;
        // If PAE is present, then load those modules.
        // ALSO, NOTE: Memory *should* be present over 4GiB to take advantage of PAE, so we
        // ensure there is. Else, we use x86.
        if((FeatureFlags & PAE_PRESENT) &&
           (BIT->HighestAddress > 0xFFFFFFFFLLU))
        {
            BIT->FileAPI(FILE_KERNEL_M, PMMX86PAE, &KernelMPMM);
            BIT->FileAPI(FILE_KERNEL_M, VMMX86PAE, &KernelMVMM);
        }

        // Else, load the x86 modules.
        else
        {
            BIT->FileAPI(FILE_KERNEL_M, PMMX86, &KernelMPMM);
            BIT->FileAPI(FILE_KERNEL_M, VMMX86, &KernelMVMM);
        }
    }

    // We shouldn't reach here.
    for(;;)
        __asm__ __volatile__("hlt");
}
