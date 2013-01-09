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

from Config import Config
from Buildmanager import BuildManager

# Parse command line parameters.
Cfg = Config()
Cfg.Parse(ARGUMENTS)

# Create an Environment with the toolchain for building.
BldMgr = BuildManager(Cfg)
Env = BldMgr.CreateEnv()

# The background image.
Env["BACK"] = "Background.bmp"

# Run through the SConscript files.
Utils = SConscript(dirs=["Utilities"], exports=["Env"])
Source = SConscript(dirs=["Source"], exports=["Env"])

# The BootImage.comp.
Image = Env.Image("BootImage.comp", None)

# The *common* binary targets.
Env["COMMON_TARGETS"] = [Env["BIOS"],
                         Env["DBAL"],
                         Env["KL"],
                         Env["Kernelx86"],
                         Env["Kernelx86_64"],
                         Env["PMMx86"],
                         Env["PMMx86_64"],
                         Env["VMMx86"],
                         Env["VMMx86PAE"],
                         Env["VMMx86_64"]]

# If the arch's i586, remove the unneccessary stuff.
if Env["ARCH"] == "i586":
    Env["COMMON_TARGETS"].remove(Env["Kernelx86_64"])
    Env["COMMON_TARGETS"].remove(Env["PMMx86_64"])
    Env["COMMON_TARGETS"].remove(Env["VMMx86_64"])

# It depends upon all the common binaries.
Depends(Image, [Env["CRC32"],
                Env["COMMON_TARGETS"]])

# If the background image exists, we depend upon the ToSIF utility.
if os.path.isfile(Env["BACK"]):
    Depends(Image, Env["ToSIF"])

# If we're building every target:
if Cfg.Options["target"] == "all":
    # Build the ISO.
    ISO = Env.ISO("Draumr.iso", None)

    # Build PXE binaries.
    PXE = Env.PXE(os.path.sep.join([Env["PXE_PATH"], "Stage1"]), None)

    # Build Floppy image.
    Floppy = Env.Floppy("Draumr.flp", None)

    # Set these as the default build target.
    Depends(ISO, Env["CD_STAGE1"])
    Depends(PXE, Env["PXE_STAGE1"])
    Depends(Floppy, Env["FLOPPY_STAGE1"])

    Depends([ISO, PXE, Floppy], Image)
    Default([ISO, PXE, Floppy])

# The ISO target.
elif Cfg.Options["target"] == "iso":
    # Build the ISO image.
    ISO = Env.ISO("Draumr.iso", None)
     
    # The default target.
    Depends(ISO, Env["CD_STAGE1"])
    Depends(ISO, Image)
    Default(ISO)

# The PXE target.
elif Cfg.Options["target"] == "pxe":
    # Build the PXE binaries.
    PXE = Env.PXE(os.path.sep.join([Env["PXE_PATH"], "Stage1"]), None)

    # The default target.  
    Depends(PXE, Env["PXE_STAGE1"])
    Depends(PXE, Image) 
    Default(PXE)

# The floppy disk target.
elif Cfg.Options["target"] == "floppy":
    # Build the floppy target.
    Floppy = Env.Floppy("Draumr.flp", None)

    # The default target.
    Depends(Floppy, Env["FLOPPY_STAGE1"])
    Depends(Floppy, Image)
    Default(Floppy)

# Clean respective things (hacky, but can't avoid).
# Clean the background.sif image.
Clean(["Draumr.iso", "Draumr.flp"], "Background.sif")

# Clean the images we copy to /tftpboot/.
for Target in Env["COMMON_TARGETS"]:
    Clean("/tftpboot/Stage1", "/tftpboot/" + os.path.basename(str(Target[0])))

# Clean the background.sif image.
Clean("/tftpboot/Stage1", "/tftpboot/Background.sif")