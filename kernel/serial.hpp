#pragma once

#include <stl/types.hpp>
#include <stl/string.hpp>
#include <stl/format.hpp>

namespace kernel::serial {

void init();

void put_byte(u8 value);
void put_char(char value);
void put(mat::StringView str);

template <class... Args>
void fmt(mat::StringView str, const Args&... args) {
	mat::format_to(&put_char, str, args...);
}

template <class... Args>
void fmtln(mat::StringView str, const Args&... args) {
	mat::format_to(&put_char, str, args...);
	put_char('\n');
}

}