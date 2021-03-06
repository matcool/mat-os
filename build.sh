#!/bin/sh

set -xe

mkdir -p build

# clang -masm=intel -nostdlib -Wall -Wextra -target i686-elf boot.s -c
# clang -masm=intel -nostdlib -Wall -Wextra -target i686-elf gdt.s -c
nasm -w+orphan-labels -f elf32 src/kernel/boot.asm -o build/boot.o

cd build
clang -std=c++20 -g -c -nostdlib -ffreestanding -fno-builtin -fno-exceptions -fno-rtti -O2 -pedantic \
  -Wall -Wextra -Wno-unused-const-variable \
  -target i386-elf \
  -I../src \
  -I../src/lib \
  ../src/*.cpp \
  ../src/kernel/*.cpp \
  -DMAT_OS=1 \
  -c
cd ..

clang -T linker.ld -g -o build/matos.bin -ffreestanding -O2 -nostdlib build/*.o -target i386-elf

if grub-file --is-x86-multiboot build/matos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi

mkdir -p build/isodir/boot/grub

cd build

cp matos.bin isodir/boot/matos.bin
cp ../grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o matos.iso isodir

cd ..
