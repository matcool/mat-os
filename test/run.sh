#!/bin/sh

clang++ test.cpp -o test \
	-std=c++20 -pedantic -Wall -Wextra -Wimplicit-fallthrough -I ../src \
	-fno-exceptions -g -fno-rtti -fsanitize=address && ./test && rm ./test
