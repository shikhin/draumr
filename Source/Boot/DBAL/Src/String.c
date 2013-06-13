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
 * Memmove - moves count bytes from source to destination - as if it is using a buffer to do so.
 *     void *dest   -> the destination to where to copy to.
 *     void *src    -> the source from where to copy to.
 *     size_t count -> the number of bytes to copy.
 *         
 * Returns:
 *     void *       -> the destination.
 */
void *memmove(void *dest, _CONST void *src, size_t count)
{
    if(!count)
    {
        return dest;
    }

    // See if src and destination are overlapping - and the src block lies previous to the destination block.
    if((src < dest) && (((uint32_t)src + count) > (uint32_t)dest))
    {
        // If yes, use a reverse copy.
        __asm__ __volatile__("std\n\t"    // std sets the direction flag, performing the rep movsb in reverse.
                "rep movsb\n\t"
                "cld"// Restore the direction flag.
                :: "c"(count), "S"((uint32_t)src + count), "D"((uint32_t)dest + count));
    }

    // Else, a standard memcpy (rep movsb in this specific case) should do the job well.
    else
    {
        memcpy(dest, src, count);
    }

    // Return the destination.
    return dest;
}

/*
 * Memset - sets the destination to a specified value - count many times.
 *     void *dest     -> the destination to which we set.
 *     int value      -> the value by which we clear.
 *     size_t count -> the count we clear.
 *
 * Returns:
 *     void *         -> the destination.
 */
void *memset(void *dest, int value, size_t count)
{
    if(!count)
        return dest;

    // Just do a simple rep stosb - good enough for now.
    __asm__ __volatile__("rep stosb" :: "c"(count), "D"(dest), "a"((uint32_t)value));
    return dest;
}

/*
 * Write a specified byte to the specified port.
 *     uint16_t Port  -> the port at which to write the value.
 *     uint8_t  Value -> the byte which is to be written at the port.
 */
void outb(uint16_t Port, uint8_t Value)
{
    __asm__ __volatile__("outb %1, %0" :: "dN"(Port), "a"(Value));
}

/*
 * Write a specified wordto the specified port.
 *     uint16_t Port  -> the port at which to write the value.
 *     uint16_t Value -> the word which is to be written at the port.
 */
void outw(uint16_t Port, uint16_t Value)
{
    __asm__ __volatile__("outw %1, %0" :: "dN"(Port), "a"(Value));
}

/*
 * Reads a byte from a specified port.
 *     uint16_t Port -> the port from where to read to.
 *
 * Returns:
 *     uint8_t       -> the byte read from the IO port.
 */
uint8_t inb(uint16_t Port)
{
    uint8_t Return;
    __asm__ __volatile__("inb %1, %0" : "=a" (Return) : "dN" (Port));
    return Return;
}
