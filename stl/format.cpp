#include "format.hpp"
#include "array.hpp"

using namespace STL_NS;
using namespace STL_NS::format;

FormatSpec format::parse_spec(StringView str) {
	FormatSpec spec;

	if (!str) return spec;

	if (str.peek_one() == '#') {
		str.take_one();
		spec.base_prefix = true;
	}

	if (str.peek_one() == '0') {
		str.take_one();
		spec.pad_type = PadType::Zero;
		while (is_digit(str.peek_one())) {
			spec.pad_amount *= 10;
			spec.pad_amount += str.take_one() - '0';
		}
	}

	switch (str.peek_one()) {
		case 'b':
			str.take_one();
			spec.base = 2;
			break;
		case 'o':
			str.take_one();
			spec.base = 8;
			break;
		case 'x':
			str.take_one();
			spec.base = 16;
			break;
		default: break;
	}

	return spec;
}

void format::Context::put(char ch) {
	m_func->call(ch);
}

void format::Context::put(StringView value) {
	for (auto ch : value) {
		m_func->call(ch);
	}
}

StringView format::Context::spec() const {
	return m_spec;
}

format::FormatSpec format::Context::parse_spec() const {
	return format::parse_spec(m_spec);
}

void format::format_impl(
	BaseFormattingFunction* func, StringView str, Span<const void*> args,
	Span<FormatImplArgFuncs> arg_funcs
) {
	// the index for which placeholder we are currently on
	usize index = 0;

	for (usize i = 0; i < str.size(); ++i) {
		const char cur = str[i];
		// if we're at the end of the string then assume this isnt the start of a placeholder
		if (i == str.size() - 1) {
			func->call(cur);
		} else {
			const char next = str[i + 1];
			if (cur == '{' || cur == '}') {
				if (next == cur) {
					// escaping the characters used for the placeholders is done by duplicating
					// them, so if the current character and next character are the same, skip the next one.
					++i;
					func->call(cur);
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

					if (index >= args.size()) {
						// we've gone through more placeholders than there are args, so error
						// TODO: error
						return;
					}

					Context ctx(func, specifiers);
					arg_funcs[index](ctx, args[index]);

					++index;
				} else {
					// we hit a } which wasnt actually started, so error
					// TODO: error
					return;
				}
			} else {
				func->call(cur);
			}
		}
	}
}

void format::format_integer(Context ctx, u64 value, bool is_negative) {
	const auto spec = ctx.parse_spec();

	if (is_negative) {
		ctx.put('-');
	}

	if (spec.base_prefix && spec.base != 10) {
		ctx.put('0');
		if (spec.base == 2) {
			ctx.put('b');
		} else if (spec.base == 8) {
			ctx.put('o');
		} else if (spec.base == 16) {
			ctx.put('x');
		}
	}

	static constexpr char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
		                               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	Array<char, 64> buffer;

	usize index = buffer.size();

	do {
		const auto digit = value % spec.base;
		value /= spec.base;
		buffer[--index] = digits[digit];
	} while (value != 0);

	if (spec.pad_type == format::PadType::Zero) {
		auto size = buffer.size() - index;
		for (auto i = size; i < spec.pad_amount; ++i) {
			ctx.put('0');
		}
	}

	for (; index < buffer.size(); ++index) {
		ctx.put(buffer[index]);
	}
}