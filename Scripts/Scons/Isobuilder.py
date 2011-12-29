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

