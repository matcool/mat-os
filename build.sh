#!/bin/sh

set -xe


# clang -masm=intel -nostdlib -Wall -Wextra -target i686-elf boot.s -c
# clang -masm=intel -nostdlib -Wall -Wextra -target i686-elf gdt.s -c
nasm -w+orphan-labels -f elf32 src/kernel/boot.asm -o build/boot.o

cd build
clang -std=c++20 -g -c -nostdlib -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -O2 -pedantic -masm=intel \
  -Wall -Wextra -Wno-unused-const-variable \
  -target i686-elf \
  -I../src \
  ../src/common.cpp \
  ../src/kernel/kernel.cpp \
  ../src/kernel/terminal.cpp \
  ../src/kernel/stdio.cpp \
  ../src/kernel/pic.cpp \
  ../src/kernel/gdt.cpp \
  ../src/kernel/idt.cpp \
  ../src/kernel/serial.cpp \
  -c
cd ..

clang -T linker.ld -g -o build/myos.bin -ffreestanding -O2 -nostdlib \
  build/boot.o \
  build/kernel.o \
  build/common.o \
  build/terminal.o \
  build/stdio.o \
  build/pic.o \
  build/gdt.o \
  build/idt.o \
  build/serial.o \
  -target i686-elf

if grub-file --is-x86-multiboot build/myos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi