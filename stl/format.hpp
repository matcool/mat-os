#pragma once

#include "span.hpp"
#include "stl.hpp"
#include "string.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace STL_NS {

// Specialize this class to implement formatting for a given type. The type given to this struct is
// decayed, so no need to worry about qualifiers. You must implement the following function:
// `static void format(format::ctx ctx, Type value);`
template <class Type>
struct Formatter;

namespace format {

enum class PadType : u8 {
	None,
	Zero,
};

struct FormatSpec {
	u32 pad_amount = 0;
	// either nothing, 'b', 'o' or 'x', resulting in bases 10, 2, 8 and 16 respectively
	u8 base = 10;
	// type of padding, either nothing or '0'
	PadType pad_type = PadType::None;
	// '#', will show a base prefix such as 0b or 0x
	bool base_prefix = false;
};

// Parses a format specifier. Currently only supports the following format:
// (#)?(0\d*)?([box])?
FormatSpec parse_spec(StringView str_spec);

template <class T>
concept FormatOutFunc = requires(T& func, char c) { func(c); };

// Base class for a type erased formatting output function.
struct BaseFormattingFunction {
	virtual void call(char ch) = 0;
};

class Context;

using FormatImplArgFuncs = void (*)(Context, const void*);

void format_impl(
	BaseFormattingFunction* func, StringView fmt_str, Span<const void*> args,
	Span<FormatImplArgFuncs> arg_funcs
);

// Wraps the formatting function and specifier, to be used in Formatter specializations.
class Context {
	BaseFormattingFunction* m_func = nullptr;
	StringView m_spec;

public:
	Context(BaseFormattingFunction* func, StringView spec) : m_func(func), m_spec(spec) {}

	void put(char c);
	void put(StringView str);

	// Accesses the specifiers as a string, unmodified
	[[nodiscard]] StringView spec() const;

	// Parses the specifiers into a standard FormatSpec.
	// It is recommended to save this to a variable instead of calling it over and over.
	[[nodiscard]] FormatSpec parse_spec() const;

	// Formats a string into the current context.
	template <class... Args>
	void fmt(StringView fmt_str, Args&&... args) {
		// TODO: this is very inefficient, no need to create a new formatting function..
		format_to([&](char c) { m_func->call(c); }, fmt_str, forward<Args>(args)...);
	}

	// Formats a single value into the current context, with an optionally given specifier.
	template <class Type>
	void fmt_value(Type&& value, StringView spec = ""_sv) {
		Formatter<types::decay<Type>>::format(Context(m_func, spec), forward<Type>(value));
	}
};

void format_integer(Context ctx, u64 value, bool is_negative);

}

template <>
struct Formatter<char> {
	static void format(format::Context ctx, char value) { ctx.put(value); }
};

template <>
struct Formatter<bool> {
	static void format(format::Context ctx, bool value) {
		if (ctx.spec() == "d"_sv) {
			ctx.put(value ? '1' : '0');
		} else {
			ctx.put(value ? "true"_sv : "false"_sv);
		}
	}
};

template <concepts::integral Int>
struct Formatter<Int> {
	static void format(format::Context ctx, Int value) {
		const bool is_negative = types::is_signed<Int> && value < 0;
		const u64 absolute_value = is_negative ? -value : value;

		format::format_integer(ctx, absolute_value, is_negative);
	}
};

template <class StringLike>
requires types::is_one_of<StringLike, String, StringView, const char*, char*>
struct Formatter<StringLike> {
	static void format(format::Context ctx, StringView value) { ctx.put(value); }
};

template <class Pointer>
requires(types::is_pointer<Pointer> && !types::is_one_of<Pointer, const char*, char*>)
struct Formatter<Pointer> {
	static void format(format::Context ctx, Pointer ptr) {
		ctx.fmt_value(reinterpret_cast<uptr>(ptr), "#016x");
	}
};

// Formats a given format string into an output function, which accepts each character at a time.
// The format string follows the python-like {} formatting, where {} is the placeholder for a value of any (formattable) type.
template <format::FormatOutFunc Func, class... Args>
void format_to(Func&& func, StringView str, Args... args) {
	// no formatting arguments provided, just return the whole string
	if constexpr (sizeof...(Args) == 0) {
		for (char c : str) {
			func(c);
		}
	} else {
		// much of this indirection is used to avoid doing any allocations, while still being able
		// to "index" the list of arguments based on a runtime index

		// Type-erase the formatting function by using a virtual call. This will allow Formatter
		// specializations to not have to be generic over the function type, thus giving the ability
		// to move Formatter specializations to source files.
		struct ErasedFormatFunc : format::BaseFormattingFunction {
			Func&& func;

			ErasedFormatFunc(Func&& func) : func(forward<Func>(func)) {}

			void call(char ch) override { return func(ch); }
		} out_func(forward<Func>(func));

		// array of type-erased args, for the runtime part of the code
		const void* arg_ptrs[] = { reinterpret_cast<const void*>(&args)... };

		// array of function pointers, capable of formatting each argument.
		// takes in a void* as to be type-erased
		format::FormatImplArgFuncs arg_funcs[] = { +[](format::Context ctx, const void* arg) {
			Formatter<types::decay<Args>>::format(ctx, *reinterpret_cast<const Args*>(arg));
		}... };

		// call the inner implementation, which deals with parsing the format string. since that is
		// always the same, it is split into a separate non templated function
		format::format_impl(
			&out_func, str, Span(arg_ptrs, sizeof...(Args)), Span(arg_funcs, sizeof...(Args))
		);
	}
}

// Formats arguments into a string. See `format_to` for more info.
template <class... Args>
String format_str(StringView str, Args&&... args) {
	if constexpr (sizeof...(Args) == 0) {
		// why would you do this
		return str;
	} else {
		String result;
		format_to([&](char c) { result.push(c); }, str, forward<Args>(args)...);
		return result;
	}
}

}