#!/bin/sh
set -e

GDB=""
X="-nographic"

show_help() {
	echo "Usage: qemu.sh [-gx]"
	echo "\t-g\tAttach GDB stub for remote debugging"
	echo "\t-x\tEnable VGA"
	echo "\t-h\tDisplay this usage help and exit"
}

while getopts gxh: opt; do
	case $opt in
		h)
			show_help
			exit 0
			;;
		g)
			GDB="-S -gdb tcp::1234"
			;;
		x)
			X=""
			;;
		*)
			show_help >&2
			exit 1
			;;
	esac
done

. ./iso.sh

qemu-img create disk.img 500m
qemu-system-i386 -m 512 -cdrom myos.iso -hda disk.img $X $GDB -d guest_errors
