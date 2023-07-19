#!/usr/bin/env bash

NAME=kernel.iso
BUILT_PATH=build/$NAME

QEMU=qemu-system-x86_64

if [ "$1" == "debug" ]; then
	EXTRA_ARGS="-s -S"
fi
if [ "$1" == "wsl" ]; then
	QEMU=qemu-system-x86_64w
	BUILT_PATH=$(wslpath -w "$BUILT_PATH")
	EXTRA_ARGS="-display sdl"
fi

$QEMU -M q35 -m 1G -cdrom $BUILT_PATH -boot d -serial stdio $EXTRA_ARGS
