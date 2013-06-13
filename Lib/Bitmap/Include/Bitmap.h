/* 
 * Contains bitmap handling prototypes.
 *
 * Copyright (c) 2013, Shikhin Sethi
 * All rights reserved.
 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Draumr nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BITMAP_H    /* Bitmap.h */
#define BITMAP_H

#include <Standard.h>
#include <stdint.h>

#define INDEX_BIT(bit)  ((bit) / 32)
#define OFFSET_BIT(bit) ((bit) % 32)

// The standard structure for a new bitmap.
typedef struct
{
    // The area where to keep the data for the bitmap.
    uint32_t *Data;
    // The size of the bitmap - in terms of number of bits.
    int64_t Size;

    // The first "zero" bit in the bitmap.
    int64_t FirstZero;
} Bitmap_t;

/*
 * Finds the first bit which is zero.
 *     Bitmap_t *Bitmap -> the bitmap in which to find the new first zero bit.
 *     int64_t From     -> the index from where to start looking for the first "zero bit".
 *
 * Returns:
 *     int64_t          -> the new "first zero bit". -1 if none found. 
 */
// NOTE: This finds the first zero bit for local usage.
int64_t FindFirstZero(Bitmap_t *Bitmap, int64_t From);

/*
 * Initializes a bitmap.
 *     uint32_t *Data -> the area where to keep the data for the bitmap.
 *     int64_t Size   -> the size of the new bitmap.
 *     uint32_t Seed  -> the data to initialize the bitmap to.
 *
 * Returns:
 *     Bitmap_t       -> a bitmap structure containing the newly initialized bitmap.
 */
Bitmap_t BitmapInit(uint32_t *Data, int64_t Size, uint32_t Seed);

/*
 * Set a particular bit in the bitmap.
 *     Bitmap_t *Bitmap -> the bitmap in which to set the particular bit.
 *     int64_t Index    -> the index of the bit to set.
 */
void BitmapSetBit(Bitmap_t *Bitmap, int64_t Index);

/*
 * Clears a particular bit in the bitmap.
 *     Bitmap_t *Bitmap -> the bitmap in which to clear the particular bit.
 *     int64_t Index    -> the index of the bit to clear.
 */
void BitmapClearBit(Bitmap_t *Bitmap, int64_t Index);

/*
 * Tests a particular bit in the bitmap.
 *     Bitmap_t *Bitmap -> the bitmap in which to test the particular bit.
 *     int64_t Index    -> the index of the bit to test.
 *
 * Returns:
 *     uint32_t        -> 0 if it isn't set, else, non-zero value.
 */
uint32_t BitmapTestBit(Bitmap_t *Bitmap, int64_t Index);

/*
 * Finds a zero bit. Sets and returns it if found. Else, returns -1.
 *     Bitmap_t *Bitmap -> the bitmap in which to find the particular bit.
 *
 * Returns:
 *     int64_t          -> the index of the bit found. -1 if none found.
 */
int64_t BitmapFindFirstZero(Bitmap_t *Bitmap);

/*
 * Finds a contiguous series of zero bits. Sets and returns them if found. Else, returns -1.
 *     Bitmap_t *Bitmap -> the bitmap in which to find the particular bit.
 *     int64_t Count    -> the number of contiguous bits to find.
 *
 * Returns:
 *     int64_t          -> the index of the starting of the series found. -1 if none found.
 */
int64_t BitmapFindContigZero(Bitmap_t *Bitmap, int64_t Count);

/*
 * Clears a contiguous series of zero bits. 
 *     Bitmap_t *Bitmap -> the bitmap in which to find the series of bits.
 *     int64_t From     -> the area from where to start clearing the series of bits.
 *     int64_t Count    -> the number of contiguous bits to clear.
 */
void BitmapClearContigZero(Bitmap_t *Bitmap, int64_t From, int64_t Count);

#endif    /* Bitmap.h */
