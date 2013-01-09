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

// DATA: for function 02h of CPUID.

// 0x06 to 0x0E.
Cache_t Cache06[] = { {0x400 * 8 , 4 },
                      {0x00000000, 0 },
                      {0x400 * 16, 4 },
                      {0x400 * 32, 4 },
                      {0x400 * 8 , 2 },
                      {0x00000000, 0 },
                      {0x400 * 16, 4 },
                      {0x400 * 16, 4 },
                      {0x400 * 24, 6 } };

// 0x21 to 0x30.
Cache_t Cache21[] = { {0x400 * 256, 8},
                      {0x400 * 512, 4},
                      {0x100000   , 8},
                      {0x00000000 , 0},
                      {0x200000   , 8},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x400000   , 8},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x400 * 32 , 8},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x400 * 32 , 8}, };

// 0x41 to 0x4E.
Cache_t Cache41[] = { {0x400 * 128, 4},
                      {0x400 * 256, 4},
                      {0x400 * 512, 4},
                      {0x100000   , 4},
                      {0x200000   , 4},
                      {0x400000   , 4},
                      {0x800000   , 8},
                      {0x300000   , 12},
                      {0x400000   , 16},
                      {0x600000   , 12},
                      {0x800000   , 16},
                      {0xC00000   , 12},
                      {0x1000000  , 16},
                      {0x600000   , 24} };

// 0x60 to 0x68.
Cache_t Cache60[] = { {0x400 * 16 , 8},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x400 * 8  , 4},
                      {0x400 * 16 , 4},
                      {0x400 * 32 , 4} };

// 0x78 to 0x87.
Cache_t Cache78[] = { {0x100000   , 4},
                      {0x400 * 128, 8},
                      {0x400 * 256, 8},
                      {0x400 * 512, 8},
                      {0x100000   , 8},
                      {0x200000   , 8},
                      {0x00000000 , 0},
                      {0x400 * 512, 2},
                      {0x400 * 512, 8},
                      {0x00000000 , 0},
                      {0x400 * 256, 8},
                      {0x400 * 512, 8},
                      {0x100000   , 8},
                      {0x200000   , 8},
                      {0x400 * 512, 4},
                      {0x100000   , 8} };

// 0xD0 to 0xEC.
Cache_t CacheD0[] = { {0x400 * 512, 4},
                      {0x100000   , 4},
                      {0x200000   , 4},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x100000   , 8},
                      {0x200000   , 8},
                      {0x400000   , 8},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x180000   , 12},
                      {0x300000   , 12},
                      {0x600000   , 12},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x200000   , 16},
                      {0x400000   , 16},
                      {0x800000   , 16},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0x00000000 , 0},
                      {0xC00000   , 24},
                      {0x1200000  , 24},
                      {0x1800000  , 24} };

/*
 * Uses (any) register provided by function 02h (cache descriptors) to detect biggest cache.
 *     uint32_t Register -> (Any) Register provided by function 02h.
 *
 * Returns:
 *     Initializes MaxCache for biggest cache detected.
 */
static void Function02h(uint32_t Register)
{
    // If the register's 31st bit isn't 0, then it doesn't contain valid descriptors.
    if(Register & (1 << 31))
    {
        return;
    }

    for(; Register != 0; Register >>= 8)
    {
        // The Cache indicated by this descriptor.
        Cache_t Cache = {0x00000000, 0x00000000};

        // The lowest 8 bits represent the descriptor.
        uint8_t Descriptor = (uint8_t)(Register & 0xFF);
        
        // Note: might not be the neatest way to do this, but not worth the overhead to do anything more.

        // 0x06 to 0x0E.
        if((Descriptor >= 0x06) && (Descriptor <= 0x0E))
        {
            Cache = Cache06[Descriptor - 0x06];
        }

        // 0x21 to 0x30.
        if((Descriptor >= 0x21) && (Descriptor <= 0x30))
        {
            Cache = Cache21[Descriptor - 0x21];
        }

        // 0x41 to 0x4E.
        if((Descriptor >= 0x41) && (Descriptor <= 0x4E))
        {
            Cache = Cache41[Descriptor - 0x41];
        }

        // 0x60 to 0x68.
        if((Descriptor >= 0x60) && (Descriptor <= 0x68))
        {
            Cache = Cache60[Descriptor - 0x60];
        }

        // 0x78 to 0x87.
        if((Descriptor >= 0x78) && (Descriptor <= 0x87))
        {
            Cache = Cache78[Descriptor - 0x78];
        }

        // 0xD0 to 0xEC.
        if((Descriptor >= 0xD0) && (Descriptor <= 0xEC))
        {
            Cache = CacheD0[Descriptor - 0xD0];
        }

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
      uint32_t EAX, EBX, ECX, EDX, MaxFunction, MaxExtFunction;
    EAX = EBX = ECX = EDX = 0x00000000;

    // Get the maximum supported extended function via EAX=0x80000000.
    EAX = 0x80000000;
    CPUID(EAX, EBX, ECX, EDX);

    // EAX contains the maximum extended function supported.
    MaxExtFunction = EAX;

    // Get the brand string and maximum supported function via EAX=0x00000000.
    EAX = 0x00000000;
    CPUID(EAX, EBX, ECX, EDX);

    // EAX contains the maximum function supported.
    MaxFunction = EAX;

    // Check for "GenuineIntel".
    if((EBX == 0x756E6547) &&     // "uneG"
       (EDX == 0x49656E69) &&      // "Ieni"
       (ECX == 0x6C65746E))          // "letn"
    {
        // If Function 04h, deterministic cache parameters is supported, then use it.
        if(MaxFunction >= 0x000000004)
        {
            Function04();
        }

        // Else if function 0x02 is supported (or no cache was detected), try for Function 02h, cache descriptors.
        else if((MaxFunction >= 0x00000002) || (MaxCache.Size == 0))
        {
            // Execute function 02h.
            EAX = 0x00000002;
            CPUID(EAX, EBX, ECX, EDX);

            // Maximum number of times CPUID needs to be called to get all descriptors is in lower 8 bits of EAX.
            uint32_t MaxTimes = EAX & 0xFF; 

            // Remove the number of times CPUID needs to be called - don't treat it as a descriptor.
            EAX &= ~0xFF;

            // Process each descriptor.
            Function02h(EAX);
            Function02h(EBX);
            Function02h(ECX);
            Function02h(EDX);

            // Since we have already called CPUID once, loop MaxTimes - 1.
            for(uint32_t i = 0; i < (MaxTimes - 1); i++)
            {
                // Execute function 02h.
                EAX = 0x00000002;
                CPUID(EAX, EBX, ECX, EDX);

                // Remove the number of times CPUID needs to be called - don't treat it as a descriptor.
                EAX &= ~0xFF;

                // Process each descriptor.
                Function02h(EAX);
                Function02h(EBX);
                Function02h(ECX);
                Function02h(EDX);
            }
        }
    }

    // Check for other processors here. TODO.
    else if(0)
    {

    }
}   