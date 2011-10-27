/* Contains common Serial port functions.
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

#ifndef SERIAL_H                      /* Serial.h */
#define SERIAL_H

#include <stdint.h>

// Define the timeout for now - and fix it later to use the PIT.
#define MIN_TIMEOUT 100000

// Outputs some text via the serial port interface.
// char *Fmt                          Contains the string to output.
// ...                                And other arguments.
void OutputSerial(char *Fmt, ...);

#endif                                /* Serial.h */