/*
 * Generic init point for PMM kmodule.
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
#include <Init.h>

// The biggest cache that we can detect.
Cache_t MaxCache = {0x00000000, 0x00000000};

/*
 * Uses function 04h (deterministic cache parameters) to detect biggest cache.
 *
 * Returns:
 *     Initializes MaxCache for biggest cache detected.
 */
static void Function04()
{
    // Some variables for registers.
    uint32_t EAX, EBX, ECX, EDX;

    // Get the brand string, via function EAX=0x00000000.
    EAX = EBX = ECX = EDX = 0x00000000;
                
    // Initialize EAX and ECX for CPUID.. 
    for(uint32_t i = ECX = 0x0000; ; ECX = ++i)
    {
        // The cache returned by CPUID this function.
        Cache_t Cache;

        // Set EAX to 0x0004 before calling CPUID.
        EAX = 0x0004; 

        // Call CPUID.
        CPUID(EAX, EBX, ECX, EDX);

        // If 'cache type' is null (no more caches), then break from the loop.
        if(!(EAX & 0x1F))
        {
            break;
        }

        // Cache size is given by (sets * associativity * line size * partitions) as per Intel manual.

        // Sets is given by ECX + 1. 
        Cache.Size = ECX + 1;
        // Associativity is given by EBX[31:22] + 1.
        Cache.Associativity = (EBX >> 22) + 1; 
        Cache.Size *= Cache.Associativity;
        // Partitions is given by EBX[21:12] + 1.
        Cache.Size *= ((EBX >> 12) & 0x3FF) + 1;
        // Line size is given by EBX[11:0] + 1.
        Cache.Size *= (EBX & 0xFFF) + 1;

        // If it is the biggest cache yet found, save it's size & associativity.
        if(Cache.Size > MaxCache.Size)
        {
            MaxCache.Size = Cache.Size;
            MaxCache.Associativity = Cache.Associativity;
        }

        // If both are equal in size, use maximum associativity.
        else if(Cache.Size == MaxCache.Size)
        {
            // Find out the maximum associativity.
            MaxCache.Associativity = 
                (Cache.Associativity > MaxCache.Associativity) ? Cache.Associativity : MaxCache.Associativity;
        }
    }
}

/*
 * The generic init function for the PMM kmodule.
 */
void GenericInit()
{
	// Some variables for registers.
	uint32_t EAX, EBX, ECX, EDX;

    // Get the brand string, via function EAX=0x00000000.
    EAX = EBX = ECX = EDX = 0x00000000;
    CPUID(EAX, EBX, ECX, EDX);

    // Check for "GenuineIntel".
    if((EBX == 0x756E6547) && 	// "uneG"
       (EDX == 0x49656E69) &&	// "Ieni"
       (ECX == 0x6C65746E))		// "letn"
    {
        // If Function 04h, deterministic cache parameters is supported, then use it.
        if(EAX >= 0x000000004)
        {
            Function04();
        }

        // Else, try for Function 02h, cache descriptors.
        else if(EAX >= 0x00000002)
        {

        }
    }

    // Check for other processors here. TODO.
    else if(0)
    {

    }
}   