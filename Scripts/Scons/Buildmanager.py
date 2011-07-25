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

from Isobuilder import ISOBuilder

from SCons.Variables import Variables, EnumVariable
from SCons.Environment import Environment

class BuildManager :

    def __init__(self, config) :
        self.config = config

    def create_env(self) :
        arch = self.config.get_arch()

        # Create an environment with our own tools.
        env = Environment()
        env["AS"] = "nasm"
        env["CC"] = "gcc"
        env["CPPFLAGS"] = ["-Wall", "-Werror", "-Wextra", "-Wshadow", "-Wpointer-arith", "-nostdlib", "-nodefaultlibs", "-fno-builtin", "-fomit-frame-pointer"]
        env["LINK"] = "ld" 
        env["BUILDERS"]["ISO"] = ISOBuilder
        
        # Add build specific flags
        b = self.config.get_build()

        if b == "debug" :
            env["CPPFLAGS"] += ["-O0"]
        else :
            env["CPPFLAGS"] += ["-O3"]

        # Hide the ugly compiler command lines and display nice messages.
        env["ASCOMSTR"] = "  AS    $SOURCE"
        env["CCCOMSTR"] = "  CC    $SOURCE"
        env["LINKCOMSTR"] = "  LINK  $TARGET"

        # Use LD_LIBRARY_PATH if it is specified in the environment where SCons was executed
        if "LD_LIBRARY_PATH" in os.environ :
            env["ENV"]["LD_LIBRARY_PATH"] = os.environ["LD_LIBRARY_PATH"]

        # Save some information in the environment.
        env["ARCH"] = arch

        return env
