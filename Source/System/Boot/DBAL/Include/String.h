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
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef STRING_H                      /* String.h */
#define STRING_H

#include <stdint.h>

/*
 * Memcpy - copy count bytes from source to destination.
 *     void *dest   -> the destination to where to copy to.
 *     void *src    -> the source from where to copy to.
 *     size_t count -> the number of bytes to copy.
 *
 * Returns:
 *     void *       -> the destination.
 */
void *memcpy(void *dest, void *src, uint32_t count);

/*
 * Memmove - moves count bytes from source to destination - as if it is using a buffer to do so..
 *     void *dest   -> the destination to where to copy to.
 *     void *src    -> the source from where to copy to.
 *     size_t count -> the number of bytes to copy.
 *         
 * Returns:
 *     void *       -> the destination.
 */
void *memmove(void *dest, void *src, uint32_t count);

/*
 * Memset - sets the destination to a specified value - count many times.
 *     void *dest     -> the destination to which we set.
 *     uint8_t value  -> the value by which we clear.
 *     uint32_t count -> the count we clear.
 *
 * Returns:
 *     void *         -> the destination.
 */
void *memset(void *dest, uint8_t value, uint32_t count);

/*
 * Write a specified byte to the specified port.
 *     uint16_t Port  -> the port at which to write the value.
 *     uint8_t  Value -> the byte which is to be written at the port.
 */
void outb(uint16_t Port, uint8_t Value);

/*
 * Write a specified wordto the specified port.
 *     uint16_t Port  -> the port at which to write the value.
 *     uint16_t Value -> the word which is to be written at the port.
 */
void outw(uint16_t Port, uint16_t Value);

/*
 * Reads a byte from a specified port.
 *     uint16_t Port -> the port from where to read to.
 *
 * Returns:
 *     uint8_t       -> the byte read from the IO port..
 */
uint8_t inb(uint16_t Port);

#endif
