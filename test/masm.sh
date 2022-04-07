#!/bin/sh

clang++ masm.cpp -o masm \
	-std=c++20 -pedantic -Wall -Wextra -Wimplicit-fallthrough -I ../src \
	-fno-exceptions -fno-rtti -fsanitize=address && ./masm
