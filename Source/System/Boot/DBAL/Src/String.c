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

#include <stdint.h>
#include <String.h>

// Memcpy - copy count bytes from source to destination.
// void *dest                         The destination to where to copy to.
// void *src                          The source from where to copy to.
// size_t count                       The number of bytes to copy.
//     rc
//                                    void * - the destination.
void *memcpy(void *dest, void *src, uint32_t count)
{
    if(!count)
        return dest;
    
    // Just do a simple rep movsb - good enough for now.
    __asm__ __volatile__("rep movsb" :: "c"(count), "S"(src), "D"(dest));
    return dest; 
}

// Memmove - moves count bytes from source to destination - as if it is using a buffer to do so..
// void *dest                         The destination to where to copy to.
// void *src                          The source from where to copy to.
// size_t count                       The number of bytes to copy.
//     rc
//                                    void * - the destination.
void *memmove(void *dest, void *src, uint32_t count)
{
    if(!count)
        return dest;
    
    // See if src and destination are overlapping - and the src block lies previous to the destination block.
    if((src < dest) && 
      (((uint32_t)src + count) > (uint32_t)dest))
    {
        // If yes, use a reverse copy.
        __asm__ __volatile__("std\n\t"           // std sets the direction flag, performing the rep movsb in reverse.
			     "rep movsb\n\t"
			     "cld"               // Restore the direction flag.
			     :: "c"(count), "S"((uint32_t)src + count), "D"((uint32_t)dest + count));
    }
    
    // Else, a standard memcpy (rep movsb in this specific case) should do the job well.
    else
        memcpy(dest, src, count);
    
    // Return the destination.
    return dest;
}

// Memset - sets the destination to a specified value - count many times.
// void *dest                         The destination to which we set.
// uint8_t value                      The value by which we clear.
// uint32_t count                     The count we clear.
//     rc
//                                    void * - the destination.
void *memset(void *dest, uint8_t value, uint32_t count)
{
    if(!count)
        return dest;
    
    // Just do a simple rep stosb - good enough for now.
    __asm__ __volatile__("rep stosb" :: "c"(count), "D"(dest), "a"((uint32_t)value));
    return dest; 
}

// Write a specified byte to the specified port.
// uint16_t Port                      The port at which to write the value.
// uint8_t  Value                     The byte which is to be written at the port.
void outb(uint16_t Port, uint8_t Value)
{
    __asm__ __volatile__("outb %1, %0" :: "dN"(Port), "a"(Value));
}

// Write a specified wordto the specified port.
// uint16_t Port                      The port at which to write the value.
// uint16_t Value                     The word which is to be written at the port.
void outw(uint16_t Port, uint16_t Value)
{
    __asm__ __volatile__("outw %1, %0" :: "dN"(Port), "a"(Value));
}


// Reads a byte from a specified port.
// uint16_t Port                      The port from where to read to.
//     rc
//                                    uint8_t - the byte to return.
uint8_t inb(uint16_t Port)
{
    uint8_t Return;
    __asm__ __volatile__("inb %1, %0" : "=a" (Return) : "dN" (Port));
    return Return;
}
