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

#ifndef _CPU_H
#define _CPU_H

#include <Standard.h>

#define LONG_MODE_PRESENT (1 << 0)
#define PAE_PRESENT       (1 << 1)

#define LONG_MODE_CPUID   (1 << 29)
#define PAE_CPUID         (1 << 6)

// Define a macro to simplify the CPUID inline assembly command.
#define CPUID(EAX, EBX, ECX, EDX) do { __asm__ __volatile__("cpuid": "=a"(EAX), "=b"(EBX), "=c"(ECX), "=d"(EDX) \
                                                        : "a"(EAX), "b"(EBX), "c"(ECX), "d"(EDX)); } while (0) 

/*
 * Initializes all the feature flags required by the KL.
 *
 * Returns:
 *     uint32_t -> the 32-bits containing all the feature flags (required).
 */
uint32_t CPUFeatureFlags(void);

#endif /* _CPU_H */
