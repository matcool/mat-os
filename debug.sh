#!/usr/bin/env bash

if [ $# -eq 1 ]; then
	gdb -ex "target remote :1234" -ex "set disassembly-flavor intel" -ex "b $1" -ex "continue" build/kernel
else
	gdb -ex "target remote :1234" -ex "set disassembly-flavor intel" build/kernel
fi