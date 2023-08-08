#!/usr/bin/env bash

set -e

ELF_PATH=$1
TARGET_PATH=$2
ISO_ROOT_PATH=$3

rm -rf $ISO_ROOT_PATH
mkdir -p $ISO_ROOT_PATH
cp \
    $ELF_PATH \
    ./limine.cfg \
    ./limine/limine-bios.sys \
    ./limine/limine-bios-cd.bin \
    ./limine/limine-uefi-cd.bin \
    $ISO_ROOT_PATH
cp -r ./kernel/assets $ISO_ROOT_PATH/assets
mkdir -p $ISO_ROOT_PATH/EFI/BOOT
cp ./limine/BOOT*.EFI $ISO_ROOT_PATH/EFI/BOOT/

xorriso -as mkisofs -b limine-bios-cd.bin \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    --efi-boot limine-uefi-cd.bin \
    -efi-boot-part --efi-boot-image --protective-msdos-label \
    $ISO_ROOT_PATH -o $TARGET_PATH

./limine/limine bios-install $TARGET_PATH

rm -rf $ISO_ROOT_PATH