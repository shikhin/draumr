/* Contains common definitions to output to the screen.
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

#ifndef OUTPUT_H                      /* Output.h */
#define OUTPUT_H

#include <stdint.h>
#include <BIT.h>

// Gives a buffer, of bpp being what we require, to be outputted to the screen.
// uint32_t *Buffer                   The address of the buffer to print.
// uint32_t X                         The X size for the buffer.
// uint32_t Y                         The Y size for the buffer.
void BufferOutput(uint32_t *Buffer, uint32_t X, uint32_t Y);

// Prints a buffer of 4bpp to the screen.
// uint8_t *Buffer                    The address of the buffer to print.
// uint32_t X                         The X size for the buffer.
// uint32_t Y                         The Y size for the buffer.
void BufferOutput4BPP(uint8_t *Buffer, uint32_t X, uint32_t Y);

#endif