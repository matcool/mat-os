#pragma once
#include "common.hpp"
#include <lib/string.hpp>
#include <lib/format.hpp>

void terminal_init();
void terminal_put_char(char);
void terminal_delete_char();

template <class... Args>
void terminal(const StringView& string, Args&&... args) {
	format_to([](char c) { terminal_put_char(c); }, string, args...);
}
