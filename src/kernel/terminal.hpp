#pragma once
#include "common.hpp"
#include <lib/string.hpp>
#include <lib/format.hpp>

void terminal_init(u32*, u32 width, u32 height);
void terminal_put_char(char);
void terminal_write_string(const char* string);

template <class... Args>
void terminal(const StringView& string, Args&&... args) {
	format_to([](char c) { terminal_put_char(c); }, string, args...);
}
