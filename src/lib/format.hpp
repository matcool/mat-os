#pragma once
#include "stl.hpp"
#include "string.hpp"
#include "template-utils.hpp"
#include "function.hpp"

template <class T>
struct Formatter {
	static_assert(always_false_t<T>, "Formatter unimplemented type");
};

template <>
struct Formatter<i32> {
	// TODO: add options
	static void format(FuncPtr<void, char> write, i32 value) {
		char buffer[11];
		size_t i = 11;
		const bool neg = value < 0;
		do {
			buffer[--i] = '0' + (value % 10);
			value /= 10;
		} while (value);
		if (neg) buffer[--i] = '-';
		while (i < 11)
			write(buffer[i++]);
	}
};

template <>
struct Formatter<u32> : Formatter<i32> {};

template <class... Args>
void format_to(FuncPtr<void, char> write, const StringView& string, Args... args) {
	Function<void()> partials[sizeof...(Args)] = { [&]() { Formatter<decltype(args)>::format(write, args); }... };

	size_t format_index = 0;
	for (size_t i = 0; i < string.size(); ++i) {
		const char c = string.at(i);
		if (i != string.size() - 1) {
			const auto next = string.at(i + 1);
			if (c == '{') {
				if (next == '{') {
					write('{');
					++i;
				} else if (next == '}') {
					++i;
					if (format_index < sizeof...(Args)) {
						partials[format_index++]();
					} else {
						// ?? error
					}
				} else {
					// ?? unimplemented format options
				}
				continue;
			} else if (c == '}' && next == '}') {
				write('}');
				++i;
				continue;
			}
		}
		write(c);
	}
}