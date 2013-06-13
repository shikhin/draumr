/* 
 * Contains bitmap handling definitions.
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

#include <stdint.h>
#include <stddef.h>
#include <Macros.h>
#include <Bitmap.h>

/*
 * Finds the first bit which is zero.
 *     Bitmap_t *Bitmap -> the bitmap in which to find the new first zero bit.
 *     int64_t From     -> the index from where to start looking for the first "zero bit".
 *
 * Returns:
 *     int64_t          -> the new "first zero bit". -1 if none found. 
 */
int64_t FindFirstZero(Bitmap_t *Bitmap, int64_t From)
{
    uint32_t *Data = &Bitmap->Data[INDEX_BIT(From)];
    uint32_t ClearedBit = 0;

    // Do all the 32-bits till the end.
    for(int64_t i = INDEX_BIT(From); i < INDEX_BIT(Bitmap->Size); i++, Data++)
    {
        // If all the bits are set, continue.
        if(*Data == 0xFFFFFFFF)
        {
            continue;
        }

        if(*Data == 0x00000000)
        {
            return i * 32;
        }

        // Find the first bit set starting from LSB in the inversed data.
        // And return with it.
        __asm__ __volatile__("bsf %1, %0" : "=r"(ClearedBit) : "r"(~(*Data)));

        return (i * 32) + ClearedBit;
    }

    // Since if no set bit is found, ClearedBit would remain the same, set it such that you can
    // verify if "not changed" and "changed" are different.
    ClearedBit = 0xFFFFFFFF;

    // Check the left-over bits at the end.
    if(OFFSET_BIT(Bitmap->Size))
    {
        // Find the first bit set starting from LSB in the inversed data.
        __asm__ __volatile__("bsf %1, %0" : "=r"(ClearedBit) : "r"(~(*Data)));

        if( (ClearedBit != 0xFFFFFFFF)
                && (ClearedBit < OFFSET_BIT(Bitmap->Size)))
        {
            // Return the new "first zero" bit.
            return (INDEX_BIT(Bitmap->Size) * 32) + ClearedBit;
        }
    }

    // Return -1.
    return -1;
}

/*
 * Find contiguous zero bits in the bitmap.
 *     Bitmap_t *Bitmap -> the bitmap in which to find the contiguous bits.
 *     int64_t From    -> the place from where to start looking for contiguous bits.
 *     int64_t Count   -> the number of contiguous bits to find.
 *
 * Returns:
 *     int64_t         -> the index of the contiguous bits. -1 for failure.
 */
static int64_t FindContigZero(Bitmap_t *Bitmap, int64_t From, int64_t Count)
{
    // If count is 1, return with a call to BitmapFindFirstZero
    if(Count == 1)
    {
        return FindFirstZero(Bitmap, From);
    }

    uint32_t *Data = &Bitmap->Data[INDEX_BIT(From)];
    // The number of contiguous bits found.
    uint32_t ContigBitsFound = 0;

    // The 'j' index for the bits in the dword.
    uint32_t j = 0;

    // Do all the 32-bits till the end.
    for(int64_t i = INDEX_BIT(From); i < INDEX_BIT(Bitmap->Size);
            i++, Data++, j = 0)
    {
        // If all the bits are set, continue.
        if(*Data == 0xFFFFFFFF)
        {
            ContigBitsFound = 0;
            continue;
        }

        if(!ContigBitsFound)
        {
            // Find the first bit which is zero by starting from LSB in the inversed data to find the set bit.
            // And then loop from it.
            __asm__ __volatile__("bsf %1, %0" : "=r"(j) : "r"(~(*Data)));

            // So we found one bit which is zero.
            ContigBitsFound = 1;

            // Increase the j counter.
            ++j;
        }

        // Check the rest of the bits.
        for(uint32_t BitToCheck = (1 << j); j < 32; j++, BitToCheck <<= 1)
        {
            if(*Data & BitToCheck)
            {
                ContigBitsFound = 0;
            }

            else
            {
                ContigBitsFound++;
                if(ContigBitsFound == Count)
                {
                    return ( (i * 32) + j) - (Count - 1);
                }
            }
        }
    }

    // Reset the 'j' counter.
    j = 0;

    // Check the left-over bits at the end.
    if(OFFSET_BIT(Bitmap->Size))
    {
        if(!ContigBitsFound)
        {
            // Find the first bit which is zero by starting from LSB in the inversed data to find the set bit.
            // And then loop from it.
            __asm__ __volatile__("bsf %1, %0" : "=r"(j) : "r"(~(*Data)));

            // So we found one bit which is zero.
            ContigBitsFound = 1;

            // If the clear bit is outside the boundary, then, return.
            if(j >= OFFSET_BIT(Bitmap->Size))
            {
                return -1;
            }

            // Increase the j counter.
            ++j;
        }

        // Check the rest of the bits.
        for(uint32_t BitToCheck = (1 << j); j < OFFSET_BIT(Bitmap->Size);
                j++, BitToCheck <<= 1)
        {
            if(*Data & BitToCheck)
            {
                ContigBitsFound = 0;
            }

            else
            {
                ContigBitsFound++;
                if(ContigBitsFound == Count)
                {
                    return ( (Bitmap->Size & ~31) + j) - (Count - 1);
                }
            }
        }
    }

    // Return -1.
    return -1;
}

/*
 * Initializes a bitmap.
 *     uint32_t *Data -> the area where to keep the data for the bitmap.
 *     int64_t Size   -> the size of the new bitmap.
 *     uint32_t Seed  -> the data to initialize the bitmap to.
 *
 * Returns:
 *     Bitmap_t       -> a bitmap structure containing the newly initialized bitmap.
 */
Bitmap_t BitmapInit(uint32_t *Data, int64_t Size, uint32_t Seed)
{
    Bitmap_t Bitmap;

    // Fill in the data.
    Bitmap.Data = Data;

    // Make size one based.
    Bitmap.Size = Size;

    Bitmap.FirstZero = 0;

    for(int64_t i = 0; i < INDEX_BIT(Size); i++)
    {
        Data[i] = Seed;
    }

    for(uint32_t j = 0, BitToSet = 1; j < OFFSET_BIT(Size); j++, BitToSet <<= 1)
    {
        Data[INDEX_BIT(Size)] |= BitToSet;
    }

    return Bitmap;
}

/*
 * Set a particular bit in the bitmap.
 *     Bitmap_t *Bitmap -> the bitmap in which to set the particular bit.
 *     int64_t Index    -> the index of the bit to set.
 */
void BitmapSetBit(Bitmap_t *Bitmap, int64_t Index)
{
    // If index isn't in bitmap, then return.
    if(Index >= Bitmap->Size)
    {
        return;
    }

    // Set the bit.
    Bitmap->Data[INDEX_BIT(Index)] |= (1 << OFFSET_BIT(Index));

    // If the bit we set was the first zero bit, then find the new first zero bit.
    if(Bitmap->FirstZero == Index)
    {
        Bitmap->FirstZero = FindFirstZero(Bitmap, Bitmap->FirstZero + 1);
    }
}

/*
 * Clears a particular bit in the bitmap.
 *     Bitmap_t *Bitmap -> the bitmap in which to set the particular bit.
 *     int64_t Index    -> the index of the bit to set.
 */
void BitmapClearBit(Bitmap_t *Bitmap, int64_t Index)
{
    // If index isn't in bitmap, then return.
    if(Index >= Bitmap->Size)
    {
        return;
    }

    // Clear the bit.
    Bitmap->Data[INDEX_BIT(Index)] &= ~ (1 << OFFSET_BIT(Index));

    // If the bit we cleared is lower than the current first zero, then make the cleared bit the new first zero bit.
    if( (Bitmap->FirstZero == -1) || (Index < Bitmap->FirstZero))
    {
        Bitmap->FirstZero = Index;
    }
}

/*
 * Tests a particular bit in the bitmap.
 *     Bitmap_t *Bitmap -> the bitmap in which to test the particular bit.
 *     int64_t Index    -> the index of the bit to test.
 *
 * Returns:
 *     uint32_t         -> 0 if it isn't set, else, non-zero value.
 */
uint32_t BitmapTestBit(Bitmap_t *Bitmap, int64_t Index)
{
    // If index isn't in bitmap, then return.
    if(Index >= Bitmap->Size)
    {
        return (uint32_t)NULL;
    }

    return Bitmap->Data[INDEX_BIT(Index)] & (1 << OFFSET_BIT(Index));
}

/*
 * Finds a zero bit. Sets and returns it if found. Else, returns -1.
 *     Bitmap_t *Bitmap -> the bitmap in which to find the particular bit.
 *
 * Returns:
 *     int64_t          -> the index of the bit found.
 */
int64_t BitmapFindFirstZero(Bitmap_t *Bitmap)
{
    // If no bit is zero, then return -1.
    if(Bitmap->FirstZero == -1)
    {
        return -1;
    }

    // Temporarily save the first zero bit's index.
    int64_t FirstZero = Bitmap->FirstZero;
    Bitmap->Data[INDEX_BIT(FirstZero)] |= (1 << OFFSET_BIT(FirstZero));

    // And find the new "first zero" bit.
    Bitmap->FirstZero = FindFirstZero(Bitmap, FirstZero + 1);

    // Return the first zero bit's index.
    return FirstZero;
}

/*
 * Finds a contiguous series of zero bits. Sets and returns them if found. Else, returns -1.
 *     Bitmap_t *Bitmap -> the bitmap in which to find the particular bit.
 *     int64_t Count    -> the number of contiguous bits to find.
 *
 * Returns:
 *     int64_t          -> the index of the starting of the series found. -1 if none found.
 */
int64_t BitmapFindContigZero(Bitmap_t *Bitmap, int64_t Count)
{
    // If no bit is zero, then return -1.
    if(Bitmap->FirstZero == -1)
    {
        return -1;
    }

    // Find the contiguous bits, starting from FirstZero.
    int64_t From, ContiguousBits = From = FindContigZero(Bitmap,
            Bitmap->FirstZero, Count);
    if(ContiguousBits == -1)
    {
        // If unable to find contiguous bits, then return with -1.
        return -1;
    }

    // Set the contiguous bits as one.
    uint32_t *Data = &Bitmap->Data[INDEX_BIT(From)];

    // Do the first odd bits.
    for(uint32_t j = OFFSET_BIT(From), BitToSet = 1 << j;
    // Keep looping till we reach a dword boundary or we finish the amount we need to clear.
            (j < 32) && (Count); BitToSet <<= 1, Count--, j++)
    {
        *Data |= BitToSet;
    }

    // Move to the next dword.
    Data++;

    // Do in dwords.
    for(; Count >= 32; Count -= 32)
    {
        *Data++ = 0xFFFFFFFF;
    }

    // Do the last odd bits.
    for(uint32_t BitToSet = (1 << 0); Count; BitToSet <<= 1, Count--)
    {
        *Data |= BitToSet;
    }

    // And find the new "first zero" bit.
    Bitmap->FirstZero = FindFirstZero(Bitmap, Bitmap->FirstZero);

    // Return the first zero bit's index.
    return ContiguousBits;
}

/*
 * Clears a contiguous series of zero bits. 
 *     Bitmap_t *Bitmap -> the bitmap in which to find the series of bits.
 *     int64_t From     -> the area from where to start clearing the series of bits.
 *     int64_t Count    -> the number of contiguous bits to clear.
 */
void BitmapClearContigZero(Bitmap_t *Bitmap, int64_t From, int64_t Count)
{
    // Clear the contiguous bits.
    uint32_t *Data = &Bitmap->Data[INDEX_BIT(From)];

    // If the first zero counter is -1, or it is above the area we are clearing, then update it.
    if( (Bitmap->FirstZero == -1) || (From < Bitmap->FirstZero))
    {
        Bitmap->FirstZero = From;
    }

    // Do the first odd bits.
    for(uint32_t j = OFFSET_BIT(From), BitToClear = 1 << j;
    // Keep looping till we reach a dword boundary or we finish the amount we need to clear.
            (j < 32) && (Count); BitToClear <<= 1, Count--, j++)
    {
        *Data &= ~BitToClear;
    }

    // Move to the next dword.
    Data++;

    // Do in dwords.
    for(; Count >= 32; Count -= 32)
    {
        *Data++ = 0x00000000;
    }

    // Do the last odd bits.
    for(uint32_t BitToClear = (1 << 0); Count; BitToClear <<= 1, Count--)
    {
        *Data &= ~BitToClear;
    }
}