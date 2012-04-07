/* 
 * Contains all compiler-specific macros.
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

#ifndef _COMPILER_H
#define _COMPILER_H

/* For GCC, which is the only compiler we support right now */
#ifdef __GNUC__
    // Structure and variable related.
    #define _PACKED     __attribute__((packed))
    #define _ALIGNED(x) __attribute__((aligned(x)))

    // Functions related.
    #define _INLINE     __attribute__((always_inline))
    #define _NORETURN   __attribute__((noreturn))
    #define _TARGET(x)  __attribute__((__target__(x)))

    // Optimization related.
    #define _HOT        __attribute__((hot))
    #define _COLD       __attribute__((cold))

    // Define _WORDSIZE
    #ifdef __x86_64__ 
        #define _WORDSIZE 64
    #else
        #define _WORDSIZE 32
    #endif

    #define va_list        __builtin_va_list
    #define va_start(x, y) __builtin_va_start(x, y)
    #define va_arg(x, y)   __builtin_va_arg(x, y)
    #define va_end(x)      __builtin_va_end(x)

#else
    #error "Compiler not supported."
#endif

#endif /* _COMPILER_H */

