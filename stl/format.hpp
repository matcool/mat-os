#pragma once

#include "stl.hpp"
#include "types.hpp"
#include "string.hpp"
#include "utils.hpp"

namespace STL_NS {

// TODO: maybe make a format namespace?

enum class FormatPadType : u8 {
	None,
	Zero,
};

struct FormatSpec {
	u32 pad_amount = 0;
	// either nothing, 'b', 'o' or 'x', resulting in bases 10, 2, 8 and 16 respectively
	u8 base = 10;
	// type of padding, either nothing or '0'
	FormatPadType pad_type = FormatPadType::None;
	// '#', will show a base prefix such as 0b or 0x
	bool base_prefix = false;
};

// Parses a format specifier. Currently only supports the following format:
// (#)?(0\d*)?([box])?
FormatSpec parse_format_spec(StringView str_spec);

template <class T>
concept FormatOutFunc = requires(T& func, char c) { func(c); };

template <FormatOutFunc Func, class Type>
struct Formatter;

// Invoke the formatter for a given type, with optional spec.
// If the formatter does not take in a spec, it is unused.
template <class Type, FormatOutFunc Func>
void formatter_as(Func func, Type value, StringView spec = "") {
	using Fmter = Formatter<Func, types::decay<Type>>;
	if constexpr (requires(Func func, Type value, StringView str) { Fmter::format(func, value, str); }) {
		Fmter::format(func, value, spec);
	} else {
		Fmter::format(func, value);
	}
}

template <FormatOutFunc Func>
struct Formatter<Func, bool> {
	static void format(Func func, bool value) {
		formatter_as(func, value ? "true" : "false");
	}
};

template <FormatOutFunc Func, concepts::integral Int>
struct Formatter<Func, Int> {
	static void format(Func func, Int value, StringView str_spec) {
		const auto spec = parse_format_spec(str_spec);

		types::to_unsigned<Int> absolute_value = value;
		
		if (types::is_signed<Int> && value < 0) {
			absolute_value = -value;
			func('-');
		}

		if (spec.base_prefix && spec.base != 10) {
			func('0');
			if (spec.base == 2) {
				func('b');
			} else if (spec.base == 8) {
				func('o');
			} else if (spec.base == 16) {
				func('x');
			}
		}
		
		static constexpr char digits[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

		char buffer[64];
		usize index = sizeof(buffer);

		do {
			const auto digit = absolute_value % spec.base;
			absolute_value /= spec.base;
			buffer[--index] = digits[digit];
		} while (absolute_value != 0);

		if (spec.pad_type == FormatPadType::Zero) {
			auto size = sizeof(buffer) - index;
			for (auto i = size; i < spec.pad_amount; ++i) {
				func('0');
			}
		}

		for (; index < sizeof(buffer); ++index) {
			func(buffer[index]);
		}
	}
};

template <FormatOutFunc Func, class StringLike>
requires types::is_one_of<StringLike, StringView, const char*, char*>
struct Formatter<Func, StringLike> {
	static void format(Func func, StringView value) {
		for (auto c : value) {
			func(c);
		}
	}
};

// Formats a given format string into an output function,
// which accepts each character at a time.
// The format string follows the python-like {} formatting,
// where {} is the placeholder for a value of any (formattable) type.
template <FormatOutFunc Func, class... Args>
void format_to(Func func, StringView str, Args... args) {
	// no formatting arguments provided, just return the whole string
	if constexpr (sizeof...(Args) == 0) {
		for (char c : str) {
			func(c);
		}
	} else {
		// the index for which placeholder we are currently on
		usize index = 0;

		// much of this indirection is used to avoid doing any allocations,
		// while still being able to "index" the list of arguments based on a
		// runtime index
		
		// array of type-erased args, for the runtime part of the code
		const void* const arg_ptrs[] = { reinterpret_cast<const void*>(&args)... };
		
		// array of function pointers, capable of formatting each argument
		// takes in a void* as to be type-erased
		using InnerFunc = void(*)(Func, const void*, StringView);
		InnerFunc arg_funcs[] = {
			+[](Func func, const void* arg, StringView spec) {
				formatter_as<Args>(func, *reinterpret_cast<const Args*>(arg), spec);
			}...
		};
		
		for (usize i = 0; i < str.size(); ++i) {
			const char cur = str[i];
			// if we're at the end of the string then assume
			// this isnt the start of a placeholder
			if (i == str.size() - 1) {
				func(cur);
			} else {
				const char next = str[i + 1];
				if (cur == '{' || cur == '}') {
					if (next == cur) {
						// escaping the characters used for the placeholders
						// is done by duplicating them, so if the current character
						// and next character are the same, skip the next one.
						++i;
						func(cur);
					} else if (cur == '{') {
						auto close_index = str.slice(i).find('}');
						if (close_index == usize(-1)) {
							// no closing bracket found, so error
							// TODO: error
							return;
						}

						close_index += i;
						const auto current_placeholder = str.slice(i, close_index);
						i = close_index;

						StringView specifiers = "";
						if (auto colon = current_placeholder.find(':'); colon != usize(-1)) {
							specifiers = current_placeholder.split_once(colon).second;
						}

						if (index >= sizeof...(Args)) {
							// we've gone through more placeholders then there are args, so error
							// TODO: error
							return;
						}
						arg_funcs[index](func, arg_ptrs[index], specifiers);
						++index;
					} else {
						// we hit a } which wasnt actually started, so error
						// TODO: error
						return;
					}
				} else {
					func(cur);
				}
			}
		}
	}
}

}