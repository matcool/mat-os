#pragma once
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

template <integral T>
struct Formatter<T> {
	static void format(auto write, T value, const StringView& options_str) {
		const bool neg = value < 0;
		if (neg) value = -value;

		struct {
			bool hex, bin;
			bool upper;
			bool pad_zeros;
			u32 count;
		} options{};

		for (size_t i = 0; i < options_str.size(); ++i) {
			if (options_str[i] == '0') {
				options.pad_zeros = true;
				options.count = 0;
				++i;
				while (i < options_str.size()) {
					const auto c = options_str[i++];
					if (c < '0' || c > '9') break;
					options.count *= 10;
					options.count += c - '0';
				}
				i -= 2;
			} else switch (options_str[i]) {
				case 'X':
					options.upper = true;
					[[fallthrough]];
				case 'x':
					options.hex = true;
					++i;
					break;
				case 'b':
					options.bin = true;
					++i;
					break;
			}
		}

		// big enough for decimal, hex and binary (with binary being the longest)
		// decimal: floor(log10(2) * (sizeof(T) * 8 - 1)) + 1
		// hex: sizeof(T) * 2
		// binary: sizeof(T) * 8
		char buffer[sizeof(T) * 8];
		size_t i = 0;
		do {
			if (options.hex) {
				u8 c = value & 0xF;
				c += c > 9 ? (options.upper ? 'A' : 'a') - 10 : '0';
				buffer[i] = c;
				value >>= 4;
			} else if (options.bin) {
				buffer[i] = '0' + (value & 1);
				value >>= 1;
			} else {
				buffer[i] = '0' + (value % 10);
				value /= 10;
			}
			++i;
		} while (value);
		if (neg) write('-');
		for (size_t j = i + neg; j < options.count; ++j)
			write('0');
		while (i)
			write(buffer[--i]);
	}
};

template <class T>
requires is_any_of<T, StringView, const char*, char*, String>
struct Formatter<T> {
	static void format(auto write, const StringView& value, const StringView&) {
		for (const char c : value)
			write(c);
	}
};

template <size_t N>
struct Formatter<char[N]> {
	static void format(auto write, const char (&value)[N], const StringView&) {
		// N goes up to the null terminator, which we don't care eabout
		for (size_t i = 0; i < N - 1; ++i)
			write(value[i]);
	}
};

template <>
struct Formatter<bool> {
	static void format(auto write, const bool value, const StringView&) {
		Formatter<StringView>::format(write, value ? "true"_sv : "false"_sv, ""_sv);
	}
};

template <>
struct Formatter<char> {
	static void format(auto write, const char value, const StringView&) {
		write(value);
	}
};

template <class T>
requires is_pointer<T> && (!is_any_of<T, char*, const char*>)
struct Formatter<T> {
	static void format(auto write, const T value, const StringView&) {
		Formatter<uptr>::format(write, reinterpret_cast<uptr>(value), "x"_sv);
	}
};

template <class F>
concept FormatToFunc = requires(F&& f, char c) { f(c); };

template <class F, class... Args>
requires FormatToFunc<F>
void format_to(F&& write, const StringView& string, Args&&... args) {
	// if no extra args are given just write out the raw string
	// should i remove the unused {} ?
	// i dunno
	if constexpr (sizeof...(Args) == 0) {
		for (const char c : string)
			write(c);
	} else {
		Function<void(const StringView&)> partials[sizeof...(Args)] =
			{ [&write, &args](const StringView& options) {
				Formatter<remove_cvref<decltype(args)>>::format(write, forward<decltype(args)>(args), options);
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
