#pragma once
#include "serial.hpp"

template <class... Args>
void log(const StringView& string, Args&&... args) {
	serial("[INFO] "_sv);
	serial(string, args...);
	serial("\n"_sv);
}
