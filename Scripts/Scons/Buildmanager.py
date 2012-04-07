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
import sys

from Isobuilder import ISOBuilder
from Pxebuilder import PXEBuilder
from Floppybuilder import FloppyBuilder
from Imagebuilder import ImageBuilder

from SCons.Variables import Variables, EnumVariable
from SCons.Environment import Environment

# Some Colors we'd be using.
Colors = {}
Colors['Green']  = '\033[92m'
Colors['End']    = '\033[0m'

class BuildManager :

    def __init__(self, Config) :
        self.Config = Config

    def CreateEnv(self) :
        # If the output is not a terminal, remove the colors.
        if not sys.stdout.isatty():
            for Key, Value in Colors.iteritems():
               Colors[Key] = ''     
	
	Arch = self.Config.GetArch()

        # Create an Environment with our own tools.
        Env = Environment()
        Env["AS"] = "nasm"
        Env["CC"] = "gcc"
        Env["CPPFLAGS"] = ["-std=c99", "-Wall", "-Wextra", "-pedantic", "-O2", "-Wshadow", "-Wcast-align", "-Wwrite-strings",
                           "-Wredundant-decls", "-Wnested-externs", 
                           "-Winline", "-Wno-attributes", "-Wno-deprecated-declarations", 
                           "-Wno-div-by-zero", "-Wno-endif-labels", "-Wfloat-equal", "-Wformat=2", "-Wno-format-extra-args",
                           "-Winit-self", "-Winvalid-pch",
                           "-Wmissing-format-attribute", "-Wmissing-include-dirs",
                           "-Wno-multichar",
                           "-Wno-pointer-to-int-cast", "-Wredundant-decls",
                           "-Wshadow", "-Wno-sign-compare",
                           "-Wswitch", "-Wsystem-headers", "-Wundef",
                           "-Wno-pragmas", "-Wno-unused-but-set-parameter", "-Wno-unused-but-set-variable", "-Wno-unused-result",
                           "-Wwrite-strings", "-Wdisabled-optimization", "-Werror", "-pedantic-errors", "-Wpointer-arith", "-nostdlib", "-nodefaultlibs", "-fno-builtin", "-fomit-frame-pointer"]
        Env["LINK"] = "ld" 
        Env["BUILDERS"]["ISO"] = ISOBuilder
        Env["BUILDERS"]["PXE"] = PXEBuilder
        Env["BUILDERS"]["Image"] = ImageBuilder
        Env["BUILDERS"]["Floppy"] = FloppyBuilder

        # Add build specific flags
        Build = self.Config.GetBuild()

        if Build == "debug" :
            Env["CPPFLAGS"] += ["-O0"]
        else :
            Env["CPPFLAGS"] += ["-O2"]

        # Hide the ugly compiler command lines and display nice messages.
        Env["ASCOMSTR"] = "  %s[AS]%s    $SOURCE" % (Colors['Green'], Colors['End'])
        Env["CCCOMSTR"] = "  %s[CC]%s    $SOURCE" % (Colors ['Green'], Colors['End'])
        Env["ARCOMSTR"] = "  %s[AR]%s    $SOURCE" % (Colors ['Green'], Colors['End'])
        Env["LINKCOMSTR"] = "  %s[LINK]%s  $TARGET" % (Colors['Green'], Colors['End'])
        Env["RANLIBCOMSTR"] = "  %s[RLIB]%s  $TARGET" % (Colors['Green'], Colors['End'])

        # Use LD_LIBRARY_PATH if it is specified in the Environment where SCons was executed
        if "LD_LIBRARY_PATH" in os.environ :
            Env["Env"]["LD_LIBRARY_PATH"] = os.environ["LD_LIBRARY_PATH"]

        # Save some information in the Environment.
        Env["ARCH"] = Arch

        return Env
