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

# Insert our own script path in front of the others.
import sys
import os
sys.path.insert(0, "Scripts/Scons")

# Parse command line parameters.
from Config import Config

Cfg = Config()
Cfg.Parse(ARGUMENTS)

# Create an Environment with the toolchain for building.
from Buildmanager import BuildManager
BldMgr = BuildManager(Cfg)
Env = BldMgr.CreateEnv()
Env["BACK"] = "Background.bmp"

# Run through the SConscript files.
Utils = SConscript(dirs=["Utilities"], exports=["Env"])
Source = SConscript(dirs=["Source"], exports=["Env"])
Depends(Source, Utils)

Image = Env.Image("BootImage.comp", None)
Depends(Image, Utils)
Depends(Image, Env["CRC32"])

if os.path.isfile(Env["BACK"]) :
    Depends(target, Env["ToSIF"])

Dependancies = (Env["BIOS"],
                Env["DBAL"],
                Env["KL"],
                Env["Kernelx86"],
                Env["KernelAMD64"])

if Cfg.Target == "all":
    ISO = Env.ISO("Draumr.iso", None)
    Depends(ISO, [Env["CD_STAGE1"], Dependancies, Image])

    PXE = Env.PXE("/tftpboot/Stage1", None) 
    Depends(PXE, [Env["PXE_STAGE1"], Dependancies, Image])

    Floppy = Env.Floppy("Draumr.flp", None)
    Depends(Floppy, [Env["FLOPPY_STAGE1"], Dependancies, Image])
    
    Clean("Draumr.iso", "Background.sif")
    Clean("/tftpboot/Stage1", "/tftpboot/Background.sif")
    Clean("/tftpboot/Stage1", "/tftpboot/DBAL")
    Clean("/tftpboot/Stage1", "/tftpboot/BIOS")
   
    Default([ISO, PXE, Floppy])

elif Cfg.Target == "iso" :
    ISO = Env.ISO("Draumr.iso", None)
    Depends(ISO, [Env["CD_STAGE1"], Dependancies, Image])
     
    Clean("Draumr.iso", "Background.sif")
    Default(ISO)

elif Cfg.Target == "pxe" :
    PXE = Env.PXE("/tftpboot/Stage1", None)
    Depends(PXE, [Env["PXE_STAGE1"], Dependancies, Image])

    Clean("/tftpboot/Stage1", "Background.sif")
    Clean("/tftpboot/Stage1", "/tftpboot/Background.sif")
    Clean("/tftpboot/Stage1", "/tftpboot/DBAL")
    Clean("/tftpboot/Stage1", "/tftpboot/BIOS")
   
    Default(PXE)

elif Cfg.Target == "floppy" :
    Floppy = Env.Floppy("Draumr.flp", None)
    Depends(Floppy, [Env["FLOPPY_STAGE1"], Dependancies, Image])

    Clean("Draumr.flp", "Background.sif")
    Default(Floppy)