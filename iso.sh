#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/kernel isodir/boot/kernel
cat > isodir/boot/grub/grub.cfg << EOF
set timeout=0
menuentry "myos" {
	multiboot /boot/kernel
}
EOF
grub-mkrescue -o myos.iso isodir
