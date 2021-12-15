#pragma once
#include "terminal.hpp"

template <class... Args>
void log(const StringView& string, Args... args) {
	terminal("[INFO] "_sv);
	terminal(string, args...);
	terminal("\n"_sv);
}
