/*
 * General string related definitions.
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

#include <stdint.h>
#include <String.h>

/*
 * Memcpy - copy count bytes from source to destination.
 *     void *dest   -> the destination to where to copy to.
 *     void *src    -> the source from where to copy to.
 *     size_t count -> the number of bytes to copy.
 *
 * Returns:
 *     void *       -> the destination.
 */
void *memcpy(void *dest, _CONST void *src, size_t count)
{
    if(!count)
    {
        return dest;
    }

    // Just do a simple rep movsb - good enough for now.
    __asm__ __volatile__("rep movsb" :: "c"(count), "S"(src), "D"(dest));
    return dest;
}

/*
 * Memset - sets the destination to a specified value - count many times.
 *     void *dest     -> the destination to which we set.
 *     uint8_t value  -> the value by which we clear.
 *     size_t count -> the count we clear.
 *
 * Returns:
 *     void *         -> the destination.
 */
void *memset(void *dest, uint8_t value, size_t count)
{
    if(!count)
        return dest;

    // Just do a simple rep stosb - good enough for now.
    __asm__ __volatile__("rep stosb" :: "c"(count), "D"(dest), "a"((uint32_t)value));
    return dest;
}
