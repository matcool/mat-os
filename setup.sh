#!/usr/bin/env bash

git clone https://github.com/limine-bootloader/limine.git --branch=v5.x-branch-binary --depth=1
make -C limine
