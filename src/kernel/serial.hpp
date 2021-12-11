#pragma once
#include "common.hpp"

#define COM1 0x3f8

void serial_init();
void serial_put_char(char c);
void serial_put_string(const char* str);

void serial_put_number(i32 n);
void serial_put_hex(u32 n);