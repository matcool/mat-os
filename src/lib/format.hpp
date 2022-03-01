#pragma once
#include "kernel/serial.hpp"
#include "stl.hpp"
#include "string.hpp"
#include "template-utils.hpp"
#include "function.hpp"
#include "vector.hpp"
#include "string.hpp"

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
	static void format(FuncPtr<void(char)> write, T value, const StringView& options) {
		// if (value == 0) return write('0');
		const bool neg = value < 0;
		if (neg) value = -value;
		// TODO: not this, maybe have a struct instead?
		if (options.size()) {
			if (options.at(0) == 'x') {
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
			} else if (options.at(0) == 'b') {
				char buffer[sizeof(T) * 8 + 2 + 1];
				size_t i = sizeof(buffer);
				do {
					buffer[--i] = '0' + (value & 1);
					value >>= 1;
				} while (value);
				buffer[--i] = 'b';
				buffer[--i] = '0';
				if (neg) buffer[--i] = '-';
				while (i < sizeof(buffer))
					write(buffer[i++]);
			}
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

template <class T>
requires is_any_of<T, StringView, char*, String>
struct Formatter<T> {
	static void format(FuncPtr<void(char)> write, const StringView& value, const StringView&) {
		for (const char c : value)
			write(c);
	}
};

template <size_t N>
struct Formatter<char[N]> {
	static void format(FuncPtr<void(char)> write, const char (&value)[N], const StringView&) {
		// N goes up to the null terminator, which we don't care eabout
		for (size_t i = 0; i < N - 1; ++i)
			write(value[i]);
	}
};

template <>
struct Formatter<bool> {
	static void format(FuncPtr<void(char)> write, const bool value, const StringView&) {
		Formatter<StringView>::format(write, value ? "true"_sv : "false"_sv, ""_sv);
	}
};

template <>
struct Formatter<char> {
	static void format(FuncPtr<void(char)> write, const char value, const StringView&) {
		write(value);
	}
};

template <class T>
requires is_pointer<T> && (!is_same<T, char*>)
struct Formatter<T> {
	static void format(FuncPtr<void(char)> write, const T value, const StringView&) {
		Formatter<uptr>::format(write, reinterpret_cast<uptr>(value), "x"_sv);
	}
};

template <class... Args>
void format_to(FuncPtr<void(char)> write, const StringView& string, Args&&... args) {
	// if no extra args are given just write out the raw string
	// should i remove the unused {} ?
	// i dunno
	if constexpr (sizeof...(Args) == 0) {
		for (const char c : string)
			write(c);
	} else {
		Function<void(const StringView&)> partials[sizeof...(Args)] =
			{ [&write, &args](const StringView& options) {
				Formatter<remove_cv<remove_ref<decltype(args)>>>::format(write, forward<decltype(args)>(args), options);
			}... };

		size_t format_index = 0;
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
							// for now just ignore
							return;
						}
					} else {
						++i;
						size_t format_options_start = i;
						while (next != '}') {
							if (++i >= string.size()) break; // error?
							next = string[i];
						}
						if (format_index < sizeof...(Args))
							partials[format_index++](string.sub(format_options_start, i));
					}
					continue;
				} else if (c == '}' && next == '}') {
					++i;
				}
			}
			write(c);
		}
	}
}
