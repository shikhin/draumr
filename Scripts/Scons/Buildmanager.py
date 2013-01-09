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

# Import important modules.
import os
import sys

# Import all builders.
from Isobuilder import ISOBuilder
from Pxebuilder import PXEBuilder
from Floppybuilder import FloppyBuilder
from Imagebuilder import ImageBuilder

from SCons.Environment import Environment
from SCons.Errors import StopError

# Some colors we'd be using.
Colors = {}
Colors['Green']  = '\033[92m'
Colors['Blue']   = '\033[34m'
Colors['End']    = '\033[0m'

class BuildManager:
    def __init__(self, Config):
        # Save the config (command line parameters parsed).
        self.Config = Config

    def CreateEnv(self):
        # If the output is not a terminal, remove the colors.
        if not sys.stdout.isatty():
            for Key, Value in Colors.iteritems():
               Colors[Key] = ''     

        # Get the arch.
        Arch = self.Config.Options["arch"]

        # Create an Environment with our own tools.
        Env = Environment()

        # Set the tools.
        Env["AS"] = self.Config.Options["AS"]
        Env["CC"] = self.Config.Options["CC"]
        Env["LINK"] = self.Config.Options["LINK"]
        Env["AR"] = self.Config.Options["AR"]
        Env["RANLIB"] = self.Config.Options["RANLIB"]
        
        # Set the compiler flags.
        Env["CCFLAGS"] = self.Config.Options["CCFLAGS"]

        # The assembler flags.
        Env["ASFLAGS"] = self.Config.Options["ASFLAGS"]

        # The linker flags.
        Env["LINKFLAGS"] = self.Config.Options["LINKFLAGS"]

        # Set the builders.
        Env["BUILDERS"]["ISO"] = ISOBuilder
        Env["BUILDERS"]["PXE"] = PXEBuilder
        Env["BUILDERS"]["Image"] = ImageBuilder
        Env["BUILDERS"]["Floppy"] = FloppyBuilder

        # Get the build.
        Build = self.Config.Options["build"]

        # Hide the ugly compiler command lines and display nice messages.
        Env["ASCOMSTR"]     = "  %s[AS]%s    $SOURCE" % (Colors['Green'], Colors['End'])
        Env["CCCOMSTR"]     = "  %s[CC]%s    $SOURCE" % (Colors['Green'], Colors['End'])
        Env["ARCOMSTR"]     = "  %s[AR]%s    $SOURCE" % (Colors['Green'], Colors['End'])
        Env["LINKCOMSTR"]   = "  %s[LINK]%s  $TARGET" % (Colors['Green'], Colors['End'])
        Env["RANLIBCOMSTR"] = "  %s[RLIB]%s  $TARGET" % (Colors['Green'], Colors['End'])

        # Use LD_LIBRARY_PATH if it is specified in the Environment where SCons was executed.
        if "LD_LIBRARY_PATH" in os.environ:
            Env["Env"]["LD_LIBRARY_PATH"] = os.environ["LD_LIBRARY_PATH"]

        # Save some information in the Environment.
        Env["ARCH"] = Arch
        Env["COLORS"] = Colors
        Env["PXE_PATH"] = self.Config.Options["pxepath"]

        if self.Config.Options["target"] == "all" or self.Config.Options["target"] == "pxe":
            if not os.path.exists(Env["PXE_PATH"]):
                raise StopError("The %s directory required for netboot (PXE) isn't present." % (Env["PXE_PATH"]))

        Tools = ["AS", "CC", "LINK", "AR", "RANLIB"]

        if self.Config.Options["toolset"] == "cross":
            for Tool in Tools:
                if not os.path.exists(self.Config.Options[Tool]):
                    if self.Config.Options["PREFIX"] == "./Tools":
                        os.system("./Crosstools.sh")

                    else:
                        os.system("./Crosstools.sh -p %s" % self.Config.Options["PREFIX"])

        # Return the environment.
        return Env