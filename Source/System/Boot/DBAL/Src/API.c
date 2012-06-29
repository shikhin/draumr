/* 
 * Contains functions to access the API.
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
 * - INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION - HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Standard.h>
#include <API.h>
#include <BIT.h>
#include <Output.h>
#include <BootFiles.h>

// The old video and file API - saved for access by the new API.
uint32_t (*OldVideoAPI)(uint32_t APICode, ...);
uint32_t (*OldFileAPI)(uint32_t APICode, ...);

/*
 * Initializes the API, replacing old APIs with new one's.
 */
void APIInit()
{
    // Save the old APIs.
    OldVideoAPI = BIT.Video.VideoAPI;
    OldFileAPI = BIT.FileAPI;

    // Restore them by the new ones.
    BIT.Video.VideoAPI = &VideoAPI;
    BIT.FileAPI = &FileAPI;
}

/*
 * The video API, to access VGA/VBE* functions.
 *     uint32_t -> the API code of the function.
 *     ...      -> rest of the arguments.
 *
 * Returns:
 *     uint32_t -> the value returned by the function. UNDEFINED if no value needs to be returned.
 */
uint32_t VideoAPI(uint32_t APICode, ...)
{
    va_list List;
    switch (APICode)
    {
        case VIDEO_VGA_SWITCH_MODE:
        case VIDEO_VBE_SWITCH_MODE:
        case VIDEO_VBE_GET_MODES:
            // Start a list for the arguments for this mode.
            va_start(List, APICode);

            // Retrieve the argument.
            uint32_t Arg = va_arg(List, uint32_t);

            // End the list.
            va_end(List);

            return OldVideoAPI(APICode, Arg);

        case VIDEO_VGA_PALETTE:
            return OldVideoAPI(APICode);

        case VIDEO_VBE_PALETTE:
            return OldVideoAPI(APICode);

        case VIDEO_OUTPUT_REVERT:
            OutputRevert();
            return 0;
    }

    return 0;
}

/*
 * The file API, contains both raw accesses and file reads.
 *     uint32_t -> the API code of the function.
 *     ...      -> rest of the arguments.
 *
 * Returns:
 *     uint32_t -> the value returned by the function. UNDEFINED if no value needs to be returned.
 */
uint32_t FileAPI(uint32_t APICode, ...)
{
    // Start a list for the arguments.
    va_list List;
    va_start(List, APICode);

    // Set of arguments.
    uint32_t Arg[2];

    // The pointer to the file structure.
    FILE_t *FILEPointer, FILEStruct;

    switch (APICode)
    {
        case FILE_OPEN:
            // Get the argument and close the list of args.
            Arg[0] = va_arg(List, uint32_t);
            va_end(List);

            // Call the old file API.
            return OldFileAPI(APICode, Arg[0]);

        case FILE_READ:
            // Get the list of arguments, and close the va.
            Arg[0] = va_arg(List, uint32_t);
            Arg[1] = va_arg(List, uint32_t);
            va_end(List);

            // Call the old file API.
            return OldFileAPI(APICode, Arg[0], Arg[1]);

        case FILE_CLOSE:
            // No arguments in the case of close.
            va_end(List);

            return OldFileAPI(APICode);

        case FILE_KERNEL:
            // Get the list of arguments, and close the va.
            Arg[0] = va_arg(List, uint32_t);
            Arg[1] = va_arg(List, uint32_t);

            FILEPointer = (FILE_t*)Arg[1];

            // Load the kernel file (the arch is passed as argument).
            FILEStruct = BootFilesKernel(Arg[0]);

            // Fill up the pointer to the file structure that we have been given.
            FILEPointer->Size = FILEStruct.Size;
            FILEPointer->Location = FILEStruct.Location;
            return 0;

        case FILE_KERNEL_M:
            // Get the list of arguments, and close the va.
            Arg[0] = va_arg(List, uint32_t);
            Arg[1] = va_arg(List, uint32_t);

            FILEPointer = (FILE_t*)Arg[1];

            // Load the kernel module file (the module file code is passed as argument).
            FILEStruct = BootFilesKernelM(Arg[0]);

            // Fill up the pointer to the file structure that we have been given.
            FILEPointer->Size = FILEStruct.Size;
            FILEPointer->Location = FILEStruct.Location;
            return 0;
    }

    return 0;
}
