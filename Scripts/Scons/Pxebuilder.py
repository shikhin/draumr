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
import shutil
import tempfile
import glob

from SCons.Builder import Builder
from SCons.Action import Action
from Isobuilder import _path

colors = {}
colors['cyan']   = '\033[96m'
colors['purple'] = '\033[95m'
colors['blue']   = '\033[94m'
colors['green']  = '\033[92m'
colors['yellow'] = '\033[93m'
colors['red']    = '\033[91m'
colors['end']    = '\033[0m'


def _pxe_builder(target, source, env) :
    # Create a temporary directory to build the ISO image structure.
    if not os.path.exists('/tftpboot'):
        raise StopError("The /tftpboot directory, which would contain the PXE files, isn't present.")

    # Copy the kernel to the image.
    s = "/tftpboot"
    
    stage1 = str(env["PXE_STAGE1"][0])
    bios = str(env["BIOS"][0])
    dbal = str(env["DBAL"][0])
    shutil.copy(stage1, s)
    shutil.copy(bios, s)
    shutil.copy(dbal, s)
    print("  %s[PXE]%s   %s" % (colors['blue'], colors['end'], target[0]))
    return 0

PXEBuilder = Builder(action = Action(_pxe_builder, None))

