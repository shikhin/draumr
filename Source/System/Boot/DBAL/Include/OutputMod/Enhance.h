/* 
 * Contains common definitions to enhance images and stuff.
  *
 * Copyright (c) 2012, Shikhin Sethi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ENHANCE_H                      /* Enhance.h */
#define ENHANCE_H

#include <stdint.h>

/*
 * Resizes a 24bpp image.
 *     uint8_t  *Input  -> the input buffer, which we are about to resize.
 *     uint8_t  *Output -> the output buffer, where we will store the resized buffer.
 *     uint32_t X       -> the previous X size of the image.
 *     uint32_t Y       -> the previous Y size of the image.
 *     uint32_t NewX    -> the X to be resized to.
 *     uint32_t NewY    -> the Y to be resized to.
 */
void ResizeBilinear(uint8_t *Input, uint8_t *Output, uint32_t X, uint32_t Y, uint32_t NewX, uint32_t NewY);

/*
 * Converts a buffer to the required BPP format, INTO the DrawBoard - and dithers if required too.
 *     uint8_t  *Input  -> the input buffer, which we are about to convert and/or dither.
 *     uint8_t  *Output -> the output buffer, where we will store the converted thingy.
 */
// NOTE: The Input & Output buffer would have a size of BIT.Video.XRes * BIT.Video.YRes
void Dither8BPP(uint8_t *Input, uint8_t *Output);

/*
 * Converts a buffer to the required BPP format, INTO the DrawBoard.
 *     uint8_t  *Input  -> the input buffer, which we are about to convert.
 *     uint8_t  *Output -> the output buffer, where we will store the converted thingy.
 */
// NOTE: The Input & Output buffer would have a size of BIT.Video.XRes * BIT.Video.YRes.
void Convert8BPP(uint8_t *Input, uint8_t *Output);

#endif
