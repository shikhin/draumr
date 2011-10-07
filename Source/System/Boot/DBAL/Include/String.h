/* General string related definitions.
* 
*  Copyright (c) 2011 Shikhin Sethi
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation;  either version 3 of the License, or
*  (at your option) any later version.
* 
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
* 
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef STRING_H                      /* String.h */
#define STRING_H

#include <stdint.h>

// Memcpy - copy count bytes from source to destination.
// void *dest                         The destination to where to copy to.
// void *src                          The source from where to copy to.
// size_t count                       The number of bytes to copy.
//     rc
//                                    void * - the destination.
void *memcpy(void *dest, void *src, uint32_t count);

// Memmove - moves count bytes from source to destination - as if it is using a buffer to do so..
// void *dest                         The destination to where to copy to.
// void *src                          The source from where to copy to.
// size_t count                       The number of bytes to copy.
//     rc
//                                    void * - the destination.
void *memmove(void *dest, void *src, uint32_t count);

// Write a specified byte to the specified port.
// uint16_t Port                      The port at which to write the value.
// uint8_t  Value                     The byte which is to be written at the port.
void outb(uint16_t Port, uint8_t Value);


// Write a specified wordto the specified port.
// uint16_t Port                      The port at which to write the value.
// uint16_t Value                     The word which is to be written at the port.
void outw(uint16_t Port, uint16_t Value);


// Reads a byte from a specified port.
// uint16_t Port                      The port from where to read to.
//     rc
//                                    uint8_t - the byte to return.
uint8_t inb(uint16_t Port);

#endif