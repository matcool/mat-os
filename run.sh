#!/usr/bin/env bash

NAME=kernel.iso
BUILT_PATH=build/$NAME

if [ "$1" == "debug" ]; then
	EXTRA_ARGS="-s -S"
fi

qemu-system-x86_64 -M q35 -m 1G -cdrom $BUILT_PATH -boot d -serial stdio $EXTRA_ARGS
