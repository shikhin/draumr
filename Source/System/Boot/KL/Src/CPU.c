/*
 * File containing functions for detecting CPU features.
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

#include <CPU.h>

/*
 * Initializes all the feature flags required by the KL.
 *
 * Returns:
 *     uint32_t -> the 32-bits containing all the feature flags (required).
 */
uint32_t CPUFeatureFlags()
{
    // The feature flags.
    uint32_t FeatureFlags = 0;

    // The registers.
    uint32_t EAX, EBX, ECX, EDX;
    EAX = EBX = ECX = EDX = 0;

    // Try to find out the extended levels supported.
    EAX = 0x80000000;
    CPUID(EAX, EBX, ECX, EDX);

    // If 0x80000001 is supported, try.
    if(EAX >= 0x80000001)
    {
        EAX = 0x80000001;
        CPUID(EAX, EBX, ECX, EDX);

        // Check whether long mode is supported or not.
        if(EDX & LONG_MODE_CPUID)
        {
            FeatureFlags |= LONG_MODE_PRESENT;
        }
    }

    // Set all registers to find out if PAE is supported or not.
    EAX = 0x00000001; EBX = ECX = EDX = 0;
    CPUID(EAX, EBX, ECX, EDX);

    // Check whether PAE is supported or not.
    if(EDX & PAE_CPUID)
    {
        FeatureFlags |= PAE_PRESENT;
    }

    return FeatureFlags;
}
