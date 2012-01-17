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

# Some colors we'd be using.
colors = {}
colors['cyan']   = '\033[96m'
colors['purple'] = '\033[95m'
colors['blue']   = '\033[94m'
colors['green']  = '\033[92m'
colors['yellow'] = '\033[93m'
colors['red']    = '\033[91m'
colors['end']    = '\033[0m'

def _path(p) :
    return os.path.sep.join(p)

def _iso_builder(target, source, env) :
    # Create a temporary directory to build the ISO image structure.
    d = tempfile.mkdtemp()

    # Copy the kernel to the image.
    s = _path([d, "Boot"])
    os.makedirs(s)
    stage1 = str(env["CD_STAGE1"][0])
    bios = str(env["BIOS"][0])
    dbal = str(env["DBAL"][0])
    background = str(env["BACK"])
    shutil.copy(stage1, s)
    shutil.copy(bios, s)
    shutil.copy(dbal, s)
    if env["BACK"] != 0:
        shutil.copy(background, s)

    os.system("mkisofs -b %s -quiet -input-charset ascii -boot-info-table -boot-load-size 9 -no-emul-boot -o %s %s" % ("Boot/Stage1", target[0], d))
    print("  [ISO]   %s" % (target[0]))
    # Clean up our mess. :)
    shutil.rmtree(d)

    return 0

ISOBuilder = Builder(action = Action(_iso_builder, None))

