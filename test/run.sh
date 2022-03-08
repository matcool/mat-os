#!/bin/sh

clang++ test.cpp -o test \
	-std=c++20 -pedantic -Wall -Wextra -I ../src \
	-fno-exceptions -fno-rtti && ./test && rm ./test
