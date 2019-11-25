#!/bin/sh
set -e
. ./iso.sh

qemu-system-$(./target-triplet-to-arch.sh $HOST) -m 512 -cdrom myos.iso -nographic -S -gdb tcp::1234 -d guest_errors
