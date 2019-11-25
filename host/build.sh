#!/bin/bash

DOWNLOAD_DIR="downloads"
TOOLCHAIN_DIR="toolchain"
PREFIX="$(pwd)/$TOOLCHAIN_DIR"

export PATH="$PREFIX/bin:$PATH"

TARGET="i686-elf"

BINUTILS_VERSION="2.33.1"
GCC_VERSION="9.2.0"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz"

echo $BINUTILS_SRC


function do_download_and_extract()
{
	url=$1

	fname="${url##*/}"

	[[ -e $fname ]] || wget $url

	[[ -d ${fname%.tar.gz*} ]] || tar xvfz $fname
}

mkdir -p $DOWNLOAD_DIR
cd $DOWNLOAD_DIR
do_download_and_extract $BINUTILS_URL
do_download_and_extract $GCC_URL
cd ..

mkdir -p $TOOLCHAIN_DIR/build
cd $TOOLCHAIN_DIR/build

mkdir -p binutils
cd binutils
[[ -e "Makefile" ]] || {
	../../../$DOWNLOAD_DIR/binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
	make
	make install
}
cd ..

mkdir -p gcc
cd gcc
which -- $TARGET-as || echo $TARGET-as is not in the PATH
[[ -e "Makefile" ]] || {
	../../../$DOWNLOAD_DIR/gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c
	make all-gcc
	make all-target-libgcc
	make install-gcc
	make install-target-libgcc
}

