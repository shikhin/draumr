 # Draumr build system.
 #
 # Copyright (c) 2012, Shikhin Sethi
 # All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met:
 #     * Redistributions of source code must retain the above copyright
 #       notice, this list of conditions and the following disclaimer.
 #     * Redistributions in binary form must reproduce the above copyright
 #       notice, this list of conditions and the following disclaimer in the
 #       documentation and/or other materials provided with the distribution.
 #     * Neither the name of Draumr nor the
 #       names of its contributors may be used to endorse or promote products
 #       derived from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 # DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import shutil

from SCons.Builder import Builder
from SCons.Action import Action
from Isobuilder import Path

def _floppy_builder(target, source, env) :
    # Create a temporary directory to build the Floppy image structure.
    FloppySize = 1474560
    Destination = open(str(target[0]), "wb")

    Stage1 = str(env["FLOPPY_STAGE1"][0])
    shutil.copyfileobj(open(Stage1, 'rb'), Destination)

    for CustTarget in env["COMMON_TARGETS"]:
        # Align to 512 byte boundary.

        # If alignment is needed.
        if Destination.tell() % 512:
            # Align.
            Destination.seek(512 - (Destination.tell() % 512), 1)

        # Copy the file.
        Filename = str(CustTarget[0])
        shutil.copyfileobj(open(Filename, 'rb'), Destination)

    # Seek to FloppySize - 1, and write a null byte to pad till N.
    Destination.seek(FloppySize - 1)
    Destination.write('\x00')

    # Print a nice text.
    print("  %s[FLP]%s   %s" % (env["COLORS"]['Blue'], env["COLORS"]['End'], target[0])) 
    
    return 0

FloppyBuilder = Builder(action = Action(_floppy_builder, None))
