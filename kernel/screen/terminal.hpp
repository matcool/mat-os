#pragma once

#include <stl/format.hpp>

namespace kernel::terminal {

// Prints an ascii character on screen
void type_character(char ch);

void go_to(u32 row, u32 column);

template <class... Args>
void fmt(StringView str, const Args&... args) {
	format_to(&type_character, str, args...);
}

template <class... Args>
void fmtln(StringView str, const Args&... args) {
	format_to(&type_character, str, args...);
	type_character('\n');
}

}