/*
 * Entry point for DBAL file.
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
#include <FPU.h>
#include <BIT.h>
#include <PMM.h>
#include <BootFiles.h>
#include <Output.h>
#include <Abort.h>

/*
 * Function to "jump" to the kernel loader.
 *     void (*EntryPoint)(void) -> the function pointer of the entry point.
 */
_PROTOTYPE(void GotoKL, (void (*EntryPoint)(void)));

/* 
 * The Main function for the DBAL sub-module.
 *     uint32_t *BITPointer -> the pointer to the BIT.
 */   
void Main(uint32_t *BITPointer)
{
    // Initialize the FPU, without which, we can't proceed.
    FPUInit();

    // Initialize the BIT - especially copy it to our side.
    BITInit(BITPointer);

    // Initialize the PMM.
    PMMInit();
    
    // Initialize the bouncer for the boot files.
    BootFilesInit();

    // Load the KL file.
    FILE_t KLFile = BootFilesKL();

    // If unable to do so, abort boot.
    if(!KLFile.Size)
    {
        AbortBoot("ERROR: Unable to load Kernel Loader file.");
    }

    // Initialize support for 'output'.
    OutputInit();

    // Go to the kernel loader.
    BootFileHeader_t *Header = (BootFileHeader_t*)KLFile.Location;
    GotoKL(Header->EntryPoint);

    // We shouldn't reach here.
    for(;;)
        __asm__ __volatile__("hlt");
}