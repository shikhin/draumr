/*
 * Contains common definitions to output to the screen.
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

#ifndef BLIT_H                      /* Blit.h */
#define BLIT_H

#include <stdint.h>
#include <BIT.h>

/*
 * Gives a buffer, of bpp being what we require, to be blitted to the screen.
 *     uint32_t *Buffer -> the address of the buffer to blit.
 */
void BlitBuffer(uint32_t *Buffer);

/*
 * Blits a buffer of 8bpp to the screen.
 *     uint32_t *Buffer -> the address of the buffer to blit.
 */
void BlitBuffer8BPP(uint32_t *Buffer);

/*
 * Blits a buffer of 15bpp to the screen.
 *     uint32_t *Buffer -> the address of the buffer to blit.
 */
void BlitBuffer15BPP(uint32_t *Buffer);

/* 
 * Blits a buffer of 16bpp to the screen.
 *     uint32_t *Buffer -> the address of the buffer to blit.
 */
void BlitBuffer16BPP(uint32_t *Buffer);

/*
 * Blits a buffer of 24bpp to the screen.
 *     uint32_t *Buffer -> the address of the buffer to blit.
 */
void BlitBuffer24BPP(uint32_t *Buffer);

/*
 * Blits a buffer of 32bpp to the screen.
 *     uint32_t *Buffer -> the address of the buffer to blit.
 */
void BlitBuffer32BPP(uint32_t *Buffer);

#endif								/* Blit.h */