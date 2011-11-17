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

#ifndef BLIT_H                      /* Blit.h */
#define BLIT_H

#include <stdint.h>
#include <BIT.h>

// Gives a buffer, of bpp being what we require, to be blitted to the screen.
// uint32_t *Buffer                   The address of the buffer to blit.
void BlitBuffer(uint32_t *Buffer);

// Blits a buffer of 4bpp to the screen.
// uint32_t *Buffer                   The address of the buffer to blit.
void BlitBuffer4BPP(uint32_t *Buffer);

// Blits a buffer of 8bpp to the screen.
// uint32_t *Buffer                   The address of the buffer to blit.
void BlitBuffer8BPP(uint32_t *Buffer);

#endif
