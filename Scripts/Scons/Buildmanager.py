# Draumr build system
#
# Copyright (c) 2011 Zoltan Kovacs, Shikhin Sethi
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import os
import sys

from Isobuilder import ISOBuilder
from Pxebuilder import PXEBuilder
from Floppybuilder import FloppyBuilder
from Imagebuilder import ImageBuilder

from SCons.Variables import Variables, EnumVariable
from SCons.Environment import Environment

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
        env["CPPFLAGS"] = ["-Wno-attributes", "-Wall", "-Werror", "-Wextra", "-Wshadow", "-Wpointer-arith", "-nostdlib", "-nodefaultlibs", "-fno-builtin", "-fomit-frame-pointer"]
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
