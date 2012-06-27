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

import os
import shutil
import tempfile
import glob

from SCons.Builder import Builder
from SCons.Action import Action
from Isobuilder import Path

def _floppy_builder(target, source, env) :
    # Create a temporary directory to build the Floppy image structure.
    Dir = tempfile.mkdtemp()

    Stage1 = str(env["FLOPPY_STAGE1"][0])
    BIOS = str(env["BIOS"][0])

    DBAL = str(env["DBAL"][0])
    DBALAligned = Path([Dir, "DBALAligned"])    
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (DBAL, DBALAligned))

    KL = str(env["KL"][0])
    KLAligned = Path([Dir, "KLAligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (KL, KLAligned))

    Kernelx86 = str(env["Kernelx86"][0])
    Kernelx86Aligned = Path([Dir, "Kernelx86Aligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (Kernelx86, Kernelx86Aligned))

    KernelAMD64 = str(env["KernelAMD64"][0])
    KernelAMD64Aligned = Path([Dir, "KernelAMD64Aligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (KernelAMD64, KernelAMD64Aligned))

    PMMx86 = str(env["PMMx86"][0])
    PMMx86Aligned = Path([Dir, "PMMx86Aligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (PMMx86, PMMx86Aligned))

    PMMx86PAE = str(env["PMMx86PAE"][0])
    PMMx86PAEAligned = Path([Dir, "PMMx86PAEAligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (PMMx86PAE, PMMx86PAEAligned))

    PMMAMD64 = str(env["PMMAMD64"][0])
    PMMAMD64Aligned = Path([Dir, "PMMAMD64Aligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (PMMAMD64, PMMAMD64Aligned))

    VMMx86 = str(env["VMMx86"][0])
    VMMx86Aligned = Path([Dir, "VMMx86Aligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (VMMx86, VMMx86Aligned))

    VMMx86PAE = str(env["VMMx86PAE"][0])
    VMMx86PAEAligned = Path([Dir, "VMMx86PAEAligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (VMMx86PAE, VMMx86PAEAligned))

    VMMAMD64 = str(env["VMMAMD64"][0])
    VMMAMD64Aligned = Path([Dir, "VMMAMD64Aligned"])
    os.system("dd if=%s bs=512 of=%s conv=sync > /dev/null 2>&1" % (VMMAMD64, VMMAMD64Aligned))
  
    Combined = Path([Dir, "Combined"])

    os.system("cat %s %s %s %s %s %s %s %s %s %s %s %s > %s" % (Stage1, BIOS, DBALAligned, KLAligned, Kernelx86Aligned, KernelAMD64Aligned,
						                PMMx86Aligned, PMMx86PAEAligned, PMMAMD64Aligned, VMMx86Aligned,
								VMMx86PAEAligned, VMMAMD64Aligned, Combined))

    os.system("dd if=%s ibs=1474560 count=100 of=%s conv=sync > /dev/null 2>&1" % (Combined, target[0]))

    print("  [FLP]   %s" % (target[0]) ) 
    
    shutil.rmtree(Dir)
    return 0

FloppyBuilder = Builder(action = Action(_floppy_builder, None))
