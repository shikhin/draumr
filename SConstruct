# -*- python -*-

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

# Insert our own script path in front of the others.
import sys
import os
sys.path.insert(0, "Scripts/Scons")

# Parse command line parameters.
from Config import Config

cfg = Config()
cfg.parse(ARGUMENTS)

# Create an environment with the toolchain for building.
from Buildmanager import BuildManager
bldmgr = BuildManager(cfg)
env = bldmgr.create_env()
env["BACK"] = "Background.bmp"

# Run through the SConscript files.
Utils = SConscript(dirs=["Utilities"], exports=["env"])
Source = SConscript(dirs=["Source"], exports=["env"])
Depends(Source, Utils)

image = env.Image("BootImage.comp", None)
Depends(image, Utils)
if cfg.target == "all":
    iso = env.ISO("Draumr.iso", None)
    Depends(iso, [env["CD_STAGE1"], env["BIOS"], env["DBAL"], image])

    pxe = env.PXE("/tftpboot/Stage1", None) 
    Depends(pxe, [env["PXE_STAGE1"], env["BIOS"], env["DBAL"], image])

    floppy = env.Floppy("Draumr.flp", None)
    Depends(floppy, [env["FLOPPY_STAGE1"], env["BIOS"], env["DBAL"], image])
    
    Clean("Draumr.iso", "Background.sif")
    Clean("/tftpboot/Stage1", "/tftpboot/Background.sif")
    Clean("/tftpboot/Stage1", "/tftpboot/DBAL")
    Clean("/tftpboot/Stage1", "/tftpboot/BIOS")
   
    Default([iso, pxe, floppy])

elif cfg.target == "iso" :
    iso = env.ISO("Draumr.iso", None)
    Depends(iso, [env["CD_STAGE1"], env["BIOS"], env["DBAL"], image])
     
    Clean("Draumr.iso", "Background.sif")
    Default(iso)

elif cfg.target == "pxe" :
    pxe = env.PXE("/tftpboot/Stage1", None)
    Depends(pxe, [env["PXE_STAGE1"], env["BIOS"], env["DBAL"], image])

    Clean("/tftpboot/Stage1", "Background.sif")
    Clean("/tftpboot/Stage1", "/tftpboot/Background.sif")
    Clean("/tftpboot/Stage1", "/tftpboot/DBAL")
    Clean("/tftpboot/Stage1", "/tftpboot/BIOS")
   
    Default(pxe)

elif cfg.target == "floppy" :
    floppy = env.Floppy("Draumr.flp", None)
    Depends(floppy, [env["FLOPPY_STAGE1"], env ["BIOS"], env["DBAL"], image])

    Clean("Draumr.flp", "Background.sif")
    Default(floppy)