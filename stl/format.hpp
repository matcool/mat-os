#pragma once

#include "stl.hpp"
#include "types.hpp"
#include "string.hpp"
#include "utils.hpp"

namespace STL_NS {

template <class T>
concept FormatOutFunc = requires(T& func, char c) { func(c); };

template <FormatOutFunc Func, class Type>
struct Formatter;

template <FormatOutFunc Func>
struct Formatter<Func, bool> {
	static void format(Func& func, bool value) {
		StringView str = value ? "true" : "false";
		for (char c : str) {
			func(c);
		}
	}
};

template <FormatOutFunc Func, concepts::integral Int>
struct Formatter<Func, Int> {
	static void format(Func& func, Int value, StringView spec) {
		types::to_unsigned<Int> absolute_value = value;
		
		bool negative = false;
		if (types::is_signed<Int> && value < 0) {
			negative = true;
			absolute_value = -value;
		}

		const int base = spec == "x" ? 16 : 10;
		
		static constexpr char digits[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

		char buffer[64];
		usize index = sizeof(buffer);

		do {
			const auto digit = absolute_value % base;
			absolute_value /= base;
			buffer[--index] = digits[digit];
		} while (absolute_value != 0);

		if (negative) {
			buffer[--index] = '-';
		}

		for (; index < sizeof(buffer); ++index) {
			func(buffer[index]);
		}
	}
};

template <FormatOutFunc Func>
struct Formatter<Func, StringView> {
	static void format(Func& func, StringView value) {
		for (auto c : value) {
			func(c);
		}
	}
};

template <FormatOutFunc Func, class... Args>
void format_to(Func func, StringView str, const Args&... args) {
	if constexpr (sizeof...(Args) == 0) {
		for (char c : str) {
			func(c);
		}
	} else {
		usize index = 0;
		
		const void* const arg_ptrs[] = { reinterpret_cast<const void*>(&args)... };
		
		using InnerFunc = void(*)(Func&, const void*, StringView);
		InnerFunc funcs[] = {
			+[](Func& func, const void* arg, StringView spec) {
				using Fmter = Formatter<Func, types::decay<Args>>;
				if constexpr (requires(Func& func, Args value, StringView str) { Fmter::format(func, value, str); }) {
					Fmter::format(func, *reinterpret_cast<const Args*>(arg), spec);
				} else {
					Fmter::format(func, *reinterpret_cast<const Args*>(arg));
				}
			}...
		};
		
		for (usize i = 0; i < str.size(); ++i) {
			const char cur = str[i];
			if (i == str.size() - 1) {
				func(cur);
			} else {
				const char next = str[i + 1];
				if (cur == '{' || cur == '}') {
					if (next == cur) {
						++i;
						func(cur);
					} else if (cur == '{') {
						auto close_index = str.slice(i).find('}');
						if (close_index == usize(-1)) return;

						close_index += i;
						const auto current_placeholder = str.slice(i, close_index);
						i = close_index;

						StringView specifiers = "";
						if (auto colon = current_placeholder.find(':'); colon != usize(-1)) {
							specifiers = current_placeholder.split_once(colon).second;
						}

						if (index >= sizeof...(Args)) {
							// error
							return;
						}
						funcs[index](func, arg_ptrs[index], specifiers);
						++index;
					} else {
						// error
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