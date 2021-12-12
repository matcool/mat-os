#pragma once
#include "kernel/serial.hpp"
#include "stl.hpp"
#include "string.hpp"
#include "template-utils.hpp"
#include "function.hpp"
#include "vector.hpp"

template <class T>
struct Formatter {
	static_assert(always_false_t<T>, "Formatter unimplemented type");
};

namespace {
	template <size_t size>
	inline constexpr auto _max_int_length = 0;
	template <> inline constexpr auto _max_int_length<1> = 4;
	template <> inline constexpr auto _max_int_length<2> = 6;
	template <> inline constexpr auto _max_int_length<4> = 11;
	template <> inline constexpr auto _max_int_length<8> = 20;
}

template <integral T>
struct Formatter<T> {
	static void format(FuncPtr<void, char> write, T value, const StringView& options) {
		if (value == 0) return write('0');
		const bool neg = value < 0;
		if (neg) value = -value;
		// TODO: not this, maybe have a struct instead?
		if (options.size() && options.at(0) == 'x') {
			// the +1 is for negative numbers
			char buffer[sizeof(T) * 2 + 2 + 1];
			size_t i = sizeof(buffer);
			do {
				u8 c = value & 0xF;
				c += c > 9 ? 'A' - 10 : '0';
				buffer[--i] = c;
				value >>= 4;
			} while (value);
			buffer[--i] = 'x';
			buffer[--i] = '0';
			if (neg) buffer[--i] = '-';
			while (i < sizeof(buffer))
				write(buffer[i++]);
		} else {
			constexpr auto max_size = _max_int_length<sizeof(T)>;
			char buffer[max_size];
			size_t i = max_size;
			do {
				buffer[--i] = '0' + (value % 10);
				value /= 10;
			} while (value);
			if (neg) buffer[--i] = '-';
			while (i < max_size)
				write(buffer[i++]);
		}
	}
};

template <>
struct Formatter<StringView> {
	static void format(FuncPtr<void, char> write, const StringView& value, const StringView&) {
		for (const char c : value)
			write(c);
	}
};

template <class... Args>
void format_to(FuncPtr<void, char> write, const StringView& string, Args... args) {
	Function<void(const StringView&)> partials[sizeof...(Args)] = { [&](const StringView& options) { Formatter<decltype(args)>::format(write, args, options); }... };

	size_t format_index = 0;
	Vector<char> format_options(0);
	for (size_t i = 0; i < string.size(); ++i) {
		const char c = string.at(i);
		if (i != string.size() - 1) {
			auto next = string.at(i + 1);
			if (c == '{') {
				if (next == '{') {
					write('{');
					++i;
				} else if (next == '}') {
					++i;
					if (format_index < sizeof...(Args)) {
						partials[format_index++](""_sv);
					} else {
						// ?? error
					}
				} else {
					format_options.clear();
					++i;
					while (next != '}') {
						format_options.push_back(next);
						if (++i >= string.size()) break; // error?
						next = string[i];
					}
					if (format_index < sizeof...(Args))
						partials[format_index++](StringView(format_options.data(), format_options.size()));
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
