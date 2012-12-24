#!/bin/bash

# Some color codes.
Blue='\033[34m'
Green='\033[92m'
End='\033[0m'

# Ensure colors are only there for terminal output.
if [[ ! -t 1 ]]; then
    Blue=''
    End=''
    Green=''
fi

# Make the build directory.
mkdir -p Tools

# The download function.
function download()
{
    local url=$1
    
    # Download the file.
    wget  --progress=dot -c -P Tools $url 2>&1 | grep --line-buffered "%" | sed -u -e "s,\.,,g" -e "s,\,,,g" | awk '{printf("\b\b\b\b%4s", $2)}'

    # Delete everything and show done.
    echo -ne "\b\b\b\b"
    echo -e " $Green[DONE]$End"
}

# Get the tools.

# Get binutils.
echo -ne "  $Blue[WGET]$End  Tools/binutils-2.23.1.tar.bz2,    "
download "http://ftp.gnu.org/gnu/binutils/binutils-2.23.1.tar.bz2"

# Get GCC.
echo -ne "  $Blue[WGET]$End  Tools/gcc-4.7.2.tar.bz2,    "
download "http://ftp.gnu.org/gnu/gcc/gcc-4.7.2/gcc-4.7.2.tar.bz2"

# Get NASM.
echo -ne "  $Blue[WGET]$End  Tools/nasm-2.10.06.tar.xz,    "
download "http://www.nasm.us/pub/nasm/releasebuilds/2.10.06/nasm-2.10.06.tar.xz"

# Untar them.

echo -e "  $Blue[UNTAR]$End Tools/binutils-2.23.1.tar.bz2"
tar -xf Tools/binutils-2.23.1.tar.bz2 -C Tools >/dev/null
rm Tools/binutils-2.23.1.tar.bz2

echo -e "  $Blue[UNTAR]$End Tools/gcc-4.7.2.tar.bz2"
tar -xf Tools/gcc-4.7.2.tar.bz2 -C Tools >/dev/null
rm Tools/gcc-4.7.2.tar.bz2

echo -e "  $Blue[UNTAR]$End Tools/nasm-2.10.06.tar.xz"
tar -xf Tools/nasm-2.10.06.tar.xz -C Tools >/dev/null
rm Tools/nasm-2.10.06.tar.xz

# Build the tools.

# Export some common stuff.
export PREFIX=$(readlink -f ./Tools)
export TARGET=x86_64-elf

# Install binutils.
mkdir -p Tools/build-binutils

echo -e "  $Blue[BINUT]$End Configuring"
cd Tools/build-binutils && ../binutils-2.23.1/configure --target=$TARGET --prefix=$PREFIX --disable-nls
cd ../../

echo -e "  $Blue[BINUT]$End Compiling"
make -C Tools/build-binutils all


echo -e "  $Blue[BINUT]$End Installing"
make -C Tools/build-binutils install

echo -e "  $Blue[BINUT]$End Cleaning"
rm -rf Tools/build-binutils Tools/binutils-2.23.1

# Install gcc.
mkdir -p Tools/build-gcc

echo -e "  $Blue[GCC]$End   Configuring"
export PATH=$PATH:$PREFIX/bin
cd Tools/build-gcc && ../gcc-4.7.2/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers
cd ../../

echo -e "  $Blue[GCC]$End   Compiling"
make -C Tools/build-gcc all-gcc

echo -e "  $Blue[GCC]$End   Installing"
make -C Tools/build-gcc install-gcc

echo -e "  $Blue[GCC]$End   Cleaning"
rm -rf Tools/build-gcc Tools/gcc-4.7.2

# Install NASM.
echo -e "  $Blue[NASM]$End  Configuring"
cd Tools/nasm-2.10.06 && ./configure --prefix=$PREFIX
cd ../../

echo -e "  $Blue[NASM]$End  Compiling"
make -C Tools/nasm-2.10.06/

echo -e "  $Blue[NASM]$End  Installing"
make -C Tools/nasm-2.10.06/ install
 
echo -e "  $Blue[NASM]$End  Cleaning"
rm -rf Tools/nasm-2.10.06