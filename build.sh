#!/bin/sh

set -xe


# clang -masm=intel -nostdlib -Wall -Wextra -target i686-elf boot.s -c
# clang -masm=intel -nostdlib -Wall -Wextra -target i686-elf gdt.s -c
nasm -w+orphan-labels -f elf32 src/kernel/boot.asm -o build/boot.o
nasm -w+orphan-labels -f elf32 src/kernel/gdt.asm -o build/gdt.o

clang -std=c++20 -c -nostdlib -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -O2 -pedantic -Wall -Wextra \
  -target i686-elf src/kernel/kernel.cpp -c -o build/kernel.o
clang -T linker.ld -o build/myos.bin -ffreestanding -O2 -nostdlib build/boot.o build/kernel.o build/gdt.o -target i686-elf

if grub-file --is-x86-multiboot build/myos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi