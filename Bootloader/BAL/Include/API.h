/* 
 * Contains functions to access the API.
 *
 * Copyright (c) 2013, Shikhin Sethi
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
 * - INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION - HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _API_H
#define _API_H

#include <Standard.h>

// The API codes for the video handling API.
#define VIDEO_VGA_SWITCH_MODE   0
#define VIDEO_VGA_PALETTE       1

#define VIDEO_VBE_SWITCH_MODE   10
#define VIDEO_VBE_PALETTE       11
#define VIDEO_VBE_GET_MODES     12

#define VIDEO_OUTPUT_REVERT     20

// The API codes for the file access API.
#define FILE_OPEN      0
#define FILE_READ      1
#define FILE_CLOSE     2

#define FILE_KERNEL    10
#define FILE_KERNEL_M  11

/*
 * Initializes the API, replacing old APIs with new one's.
 */
void APIInit(void) _COLD;

/*
 * The video API, to access VGA/VBE* functions.
 *     uint32_t -> the API code of the function.
 *     ...      -> rest of the arguments.
 *
 * Returns:
 *     uint32_t -> the value returned by the function. UNDEFINED if no value needs to be returned.
 */
uint32_t VideoAPI(uint32_t APICode, ...);

/*
 * The file API, contains both raw accesses and file reads.
 *     uint32_t -> the API code of the function.
 *     ...      -> rest of the arguments.
 *
 * Returns:
 *     uint32_t -> the value returned by the function. UNDEFINED if no value needs to be returned.
 */
uint32_t FileAPI(uint32_t APICode, ...);

/*
 * The abort boot services function.
 */
void AbortBootServices(void);

#endif /* _BIT_H */
