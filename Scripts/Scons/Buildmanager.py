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
import sys

from Isobuilder import ISOBuilder
from Pxebuilder import PXEBuilder
from Floppybuilder import FloppyBuilder
from Imagebuilder import ImageBuilder

from SCons.Variables import Variables, EnumVariable
from SCons.Environment import Environment

# Some colors we'd be using.
colors = {}
colors['cyan']   = '\033[96m'
colors['purple'] = '\033[95m'
colors['blue']   = '\033[94m'
colors['green']  = '\033[92m'
colors['yellow'] = '\033[93m'
colors['red']    = '\033[91m'
colors['end']    = '\033[0m'

class BuildManager :

    def __init__(self, config) :
        self.config = config

    def create_env(self) :
       #If the output is not a terminal, remove the colors
        if not sys.stdout.isatty():
            for key, value in colors.iteritems():
               colors[key] = ''     
	
	arch = self.config.get_arch()

        # Create an environment with our own tools.
        env = Environment()
        env["AS"] = "nasm"
        env["CC"] = "gcc"
        env["CPPFLAGS"] = ["-std=c99", "-march=i486", "-Wno-attributes", "-Wall", "-Werror", "-Wextra", "-Wshadow", "-Wpointer-arith", "-nostdlib", "-nodefaultlibs", "-fno-builtin", "-fomit-frame-pointer"]
        env["LINK"] = "ld" 
        env["BUILDERS"]["ISO"] = ISOBuilder
        env["BUILDERS"]["PXE"] = PXEBuilder
        env["BUILDERS"]["Image"] = ImageBuilder
        env["BUILDERS"]["Floppy"] = FloppyBuilder

        # Add build specific flags
        b = self.config.get_build()

        if b == "debug" :
            env["CPPFLAGS"] += ["-O0"]
        else :
            env["CPPFLAGS"] += ["-O2"]

        # Hide the ugly compiler command lines and display nice messages.
        env["ASCOMSTR"] = "  %s[AS]%s    $SOURCE" % (colors['green'], colors['end'])
        env["CCCOMSTR"] = "  %s[CC]%s    $SOURCE" % (colors ['green'], colors['end'])
        env["LINKCOMSTR"] = "  %s[LINK]%s  $TARGET" % (colors['green'], colors['end'])

        # Use LD_LIBRARY_PATH if it is specified in the environment where SCons was executed
        if "LD_LIBRARY_PATH" in os.environ :
            env["ENV"]["LD_LIBRARY_PATH"] = os.environ["LD_LIBRARY_PATH"]

        # Save some information in the environment.
        env["ARCH"] = arch

        return env
