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