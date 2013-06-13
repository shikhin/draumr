/*
 * Definitions of functions to open, close and read boot files.
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

#ifndef _BOOT_FILES_H
#define _BOOT_FILES_H

#include <Standard.h>

// Define the "file code" of every file, for easy usage.
#define BACKGROUND_SIF 0x02
#define KL             0x03
#define KERNEL_X86     0x04
#define KERNEL_X86_64   0x05
#define PMM_X86         0x06
#define PMM_X86_64       0x07
#define VMM_X86         0x08
#define VMM_X86PAE      0x09
#define VMM_X86_64       0x0A

// Architecture.
#define ARCH_X86       0x00
#define ARCH_PAE       0x01
#define ARCH_X86_64     0x02

// The "standard" location of some files.
#define KL_LOCATION    0x18000

// Bouncer has size of 64KiB.
#define BOUNCER_SIZE        (64 * 1024)
#define BOUNCER_SMALL_SIZE  (16 * 1024)

// Pages used by bouncer.
#define BOUNCER_PAGES       ((64 * 1024)/4096)
#define BOUNCER_SMALL_PAGES ((16 * 1024)/4096)

// The SIF image file format's header.
struct SIFHeader
{
    uint8_t Type[3];
    uint32_t FileSize;
    uint32_t CRC32;
    uint32_t Offset;

    uint16_t Plane, BPP;
    uint32_t Compression, ImageSize;
    uint32_t XRes, YRes;
}_PACKED;

// The Boot File generic header format.
struct BootFileHeader
{
    uint8_t Signature[4];
    void (*EntryPoint)(void);
    uint32_t FileStart;
    uint32_t FileEnd;

    uint32_t BSSStart;
    uint32_t BSSEnd;

    uint32_t CRC32;
}_PACKED;

// The Kernel generic header format.
struct KernelHeader
{
    uint8_t Signature[4];
    uint64_t EntryPoint;
    uint64_t FileStart;
    uint64_t FileEnd;

    uint64_t BSSStart;
    uint64_t BSSEnd;

    uint32_t CRC32;
}_PACKED;

// The kernel modules generic header format.
struct KernelMHeader
{
    uint8_t Signature[4];
    uint64_t EntryPoint;
    uint64_t FileStart;
    uint64_t FileEnd;

    uint64_t BSSStart;
    uint64_t BSSEnd;

    uint32_t CRC32;
}_PACKED;

// The file structure, which is returned by opening files.
typedef struct
{
    // The size of the file.
    uint32_t Size;

    // The location of the file.
    void *Location;
} FILE_t;

// Some typedef's to make stuff easier.
typedef struct SIFHeader SIFHeader_t;
typedef struct BootFileHeader BootFileHeader_t;
typedef struct KernelHeader KernelHeader_t;
typedef struct KernelMHeader KernelMHeader_t;

/*
 * Initializes the bouncer in which we would be reading the required boot files.
 */void BootFilesInit(void) _COLD;

/*
 * Clears up everything initialized in the Init().
 */
void BootFilesClear(void);

/*
 * Gets the background image, verifying what we are getting too.
 *
 * Returns:
 *     FILE_t -> the file structure containing address and length of the file.
 */
FILE_t BootFilesBGImg(void);

/*
 * Gets the Kernel Loader file, verifying what we are getting too.
 *
 * Returns:
 *     FILE_t -> the file structure containing address and length of the file.
 *     Boot   -> aborts boot if unable to load the KL.
 */
FILE_t BootFilesKL(void);

/*
 * Gets the kernel, verifying what we are getting too.
 *     uint32_t Arch -> the arch of the kernel to load.
 *
 *                          0x01 -> x86
 *                          0x02 -> x86_64
 *
 * Returns:
 *     FILE_t        -> the file structure containing address and length of the file.
 *     Boot          -> aborts boot if we are unable to load the kernel.
 */
FILE_t BootFilesKernel(uint32_t Arch);

/*
 * Gets the kernel modules, verifying what we are getting too.
 *     uint32_t ModuleFileCode -> the file code of the module.
 *
 * Returns:
 *     FILE_t                  -> the file structure containing address and length of the file.
 *     Boot                    -> aborts boot if we are unable to load the kernel module.
 */
FILE_t BootFilesKernelM(uint32_t ModuleFileCode);

#endif /* _BOOT_FILES_H */
