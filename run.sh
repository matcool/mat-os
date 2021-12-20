#!/bin/sh

set -xe

# This command is strictly MAT only, not being MAT will make this
# command not work. So therfore you non MATs will have to edit the
# qemu executable path.

# -d int -M smm=off
./build.sh && "/mnt/c/Program Files/qemu/qemu-system-i386.exe" -display sdl -s -serial stdio -cdrom build/matos.iso
