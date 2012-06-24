#!/bin/bash

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

# Only one argument.
ARGS=2

# Some variables - which we'd be testing case.
PXE="pxe"
CD="iso"
Floppy="floppy"
Clean="clean"
Release="release"
Build="debug"

for Var in "$@"
do
    if [ "$Var" == "$Clean" ]
    then
        echo "Cleaning."
        scons -j4 -c
        exit
        
    elif [ "$Var" == "$Release" ]
    then
        echo "Release build."
        Build="release"
        
    fi
done

for Var in "$@"
do
	if [ "$Var" == "$PXE" ] 
    then
        echo "Compiling for PXE."
        scons -j4 target=pxe build="$Build"

        echo -e "\nOpening QEMU, using PXE."
        qemu-system-i386 -s -monitor stdio -tftp /tftpboot/ -bootp /Stage1 -boot n -net user -net nic,model=rtl8139,macaddr=00:11:22:33:44:55

    elif [ "$Var" == "$Floppy" ] 
    then
        echo "Compiling for Floppy."
	    scons -j4 target=floppy build="$Build"

        # Remove the Draumr iso file, in case it's already there.
        rm Draumr.iso
        
        echo -e "\nOpening QEMU, using floppy."
	    qemu-system-i386 -s -monitor stdio -fda Draumr.flp

    elif [ "$Var" == "$CD" ] 
    then
        echo "Compiling for CD."
        scons -j4 target=iso build="$Build"
       
        rm Draumr.flp
        
        echo -e "\nOpening QEMU, using CD."
        qemu-system-i386 -s -monitor stdio -cdrom Draumr.iso

    fi
done

