#!/usr/bin/env bash

qemu-system-x86_64 -M q35 -m 2G -cdrom build/kernel.iso -boot d -serial stdio -display sdl