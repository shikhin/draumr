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
from Config import Config

def _image_builder(target, source, env) :    
    os.system("%s %s" % (env["CRC32"][0], env["BIOS"][0]))
    os.system("%s %s" % (env["CRC32"][0], env["DBAL"][0]))
    
    if os.path.isfile(env["BACK"]) :
        os.system("%s %s Background.sif" % (env["ToSIF"][0], env["BACK"]))
        env["BACK"] = "Background.sif"              
        
    else :
        env["BACK"] = 0
        
    return 0

ImageBuilder = Builder(action = Action(_image_builder, None))

