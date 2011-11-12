# -*- python -*-

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

# Insert our own script path in front of the others.
import sys
sys.path.insert(0, "Scripts/Scons")

# Parse command line parameters.
from Config import Config

cfg = Config()
cfg.parse(ARGUMENTS)

# Create an environment with the toolchain for building.
from Buildmanager import BuildManager
bldmgr = BuildManager(cfg)
env = bldmgr.create_env()

# Run through the SConscript files.
Utils = SConscript(dirs=["Utilities"], exports=["env"])
Source = SConscript(dirs=["Source"], exports=["env"])
Depends(Source, Utils)

image = env.Image("BootImage.comp", None)
if cfg.target == "all":
    iso = env.ISO("Draumr.iso", None)
    Depends(iso, [env["CD_STAGE1"], env["BIOS"], env["DBAL"], image])

    pxe = env.PXE("/tftpboot/Stage1", None) 
    Depends(pxe, [env["PXE_STAGE1"], env["BIOS"], env["DBAL"], image])

    floppy = env.Floppy("Draumr.flp", None)
    Depends(floppy, [env["FLOPPY_STAGE1"], env["BIOS"], env["DBAL"], image])

    Default([iso, pxe, floppy])

elif cfg.target == "iso" :
    iso = env.ISO("Draumr.iso", None)
    Depends(iso, [env["CD_STAGE1"], env["BIOS"], env["DBAL"], image])
     
    Default(iso)

elif cfg.target == "pxe" :
    pxe = env.PXE("/tftpboot/Stage1", None)
    Depends(pxe, [env["PXE_STAGE1"], env["BIOS"], env["DBAL"], image])

    Default(pxe)

elif cfg.target == "floppy" :
    floppy = env.Floppy("Draumr.flp", None)
    Depends(floppy, [env["FLOPPY_STAGE1"], env ["BIOS"], env["DBAL"], image])

    Default(floppy)
