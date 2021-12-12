#pragma once
#include "common.hpp"
#include <lib/string.hpp>
#include <lib/format.hpp>

#define COM1 0x3f8

void serial_init();
void serial_put_char(char c);
void serial_put_string(const char* str);

void serial_put_number(i32 n);
void serial_put_hex(u32 n);

template <class... Args>
void serial(const StringView& string, Args... args) {
	format_to([](char c) { serial_put_char(c); }, string, args...);
}