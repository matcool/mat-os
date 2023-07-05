#!/usr/bin/env bash

gdb -ex "target remote :1234" -ex "set disassembly-flavor intel" build/kernel