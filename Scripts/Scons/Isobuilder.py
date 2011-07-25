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

def _path(p) :
    return os.path.sep.join(p)

def _iso_builder(target, source, env) :
    # Create a temporary directory to build the ISO image structure.
    d = tempfile.mkdtemp()

    # Copy the kernel to the image.
    s = _path([d, "Boot"])
    os.makedirs(s)
    stage1 = str(env["CD_STAGE1"][0])
    bios = "Source/System/Boot/BIOS/CD/BIOS"
    shutil.copy(stage1, s)
    shutil.copy(bios, s)

    os.system("mkisofs -b %s -boot-info-table -boot-load-size 4 -no-emul-boot -o %s %s" % ("Boot/Stage1", target[0], d))

    # Clean up our mess. :)
    shutil.rmtree(d)

    return 0

ISOBuilder = Builder(action = Action(_iso_builder, None))

