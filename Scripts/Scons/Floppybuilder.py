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
 #     * Neither the name of the <organization> nor the
 #       names of its contributors may be used to endorse or promote products
 #       derived from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 # DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import shutil
import tempfile
import glob

from SCons.Builder import Builder
from SCons.Action import Action
from Isobuilder import Path

def _floppy_builder(Target, Source, Env) :
    # Create a temporary directory to build the Floppy image structure.
    Dir = tempfile.mkdtemp()

    Stage1 = str(Env["FLOPPY_STAGE1"][0])
    BIOS = str(Env["BIOS"][0])
    DBAL = str(Env["DBAL"][0])
    
    Combined1 = Path([Dir, "Combined"])
    Combined2 = Path([Dir, "Combined2"])

    os.system("cat %s %s > %s" % (Stage1, BIOS, Combined1))
    os.system("cat %s %s > %s" % (Combined1, DBAL, Combined2))
    os.system("dd if=%s ibs=1474560 count=100 of=%s conv=sync > /dev/null 2>&1" % (Combined2, Target[0]))

    print("  [FLP]   %s" % (Target[0]) ) 
    
    shutil.rmtree(Dir)
    return 0

FloppyBuilder = Builder(action = Action(_floppy_builder, None))