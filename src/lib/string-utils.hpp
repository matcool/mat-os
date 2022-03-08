#pragma once
#include "stl.hpp"
#include "string.hpp"

template <class T>
T parse_int(const StringView& str, const u8 base = 10) {
	T value = 0;
	for (const auto c : str) {
		if (c >= '0' && c <= '9') {
			value *= base;
			value += c - '0';
		} else if (c >= 'a' && c <= 'z') {
			value *= base;
			value += c - 'a' + 10;
		} else if (c >= 'A' && c <= 'Z') {
			value *= base;
			value += c - 'A' + 10;
		} else
			break;
	}
	return value;
}
