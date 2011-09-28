/* General BIT related definitions and structures.
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
#include <BIT.h>

// Define the BIT structure here.
BIT_t BIT;

// Initializes the BIT structure, copying it to somewhere appropriate.
// uint32_t *BITPointer               The pointer to the BIT structure, as passed to us.
void BITInit(uint32_t *BITPointer)
{
    memcpy(&BIT, BITPointer, sizeof(BIT_t));
    return; 
}