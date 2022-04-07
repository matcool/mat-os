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

// TODO: make it return result?
template <class T>
T parse_int_literal(StringView str) {
	T value = 0;
	const auto size = str.size();
	u8 base = 10;
	if (size == 0) return value;
	if (str[0] == '0') {
		if (size >= 3) {
			switch (str[1]) {
				case 'x':
				case 'X':
					base = 16;
					break;
				case 'b':
				case 'B':
					base = 2;
					break;
				case 'o':
				case 'O':
					base = 8;
					break;
				// uhh error
			}
			str = str.sub(2);
		} else {
			// uhh error
		}
	}

	return parse_int<T>(str, base);
}
