#!/usr/bin/env bash

NAME=kernel.iso
BUILT_PATH=build/$NAME

if [ $(uname -r | sed -n 's/.*\( *Microsoft *\).*/\1/ip') ]; then
	BUILT_PATH=$(wslpath -w "$BUILT_PATH")
fi

qemu-system-x86_64 -M q35 -m 1G -cdrom $BUILT_PATH -boot d -serial stdio -display sdl
