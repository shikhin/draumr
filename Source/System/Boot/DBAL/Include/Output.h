/* Contains common Output definitions.
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

#ifndef VIDEO_H                       /* Video.h */
#define VIDEO_H

#include <stdint.h>

// Define the timeout for now - and fix it later to use the PIT or something.
#define MIN_TIMEOUT 100000

// Intializes a proper video mode, which is supported by the OS, the video card and the monitor (and is beautiful).
// If no video card, initializes the serial port.
void OutputInit();

#endif                                /* Video.h */