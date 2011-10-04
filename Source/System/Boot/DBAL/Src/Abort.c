/* Contains functions to Abort boot.
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

#ifndef ABORT_H                       /* Abort.h */
#define ABORT_H

#include <stdint.h>
#include <Log.h>
#include <String.h>

// Aborts boot, by giving a endless beep, and trying to print a message on the screen.
// char *String                       The message to print.
void AbortBoot(char *String)
{
    DebugPrintText(String);           // Print the message using the text version. 

    outb(0x42, 0xB6);
    outb(0x42, 0xD1);                 // Send lower 16-bits of count for frequency to play.            
    outb(0x42, 0x11);                 // Send higher 16-bits of count for frequency to play.
       
    outb(0x61, (inb(0x61) | 0x3));    // Set the Speaker enable and other required bit - SPEAK. 

    // And halt forever in the dark shadowy land of Motherboardy.
    for(;;)
        __asm__ __volatile__("hlt");
}

#endif                                /* Abort.h */