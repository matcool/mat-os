#include "format.hpp"

namespace STL_NS {

FormatSpec parse_format_spec(StringView str) {
	FormatSpec spec;

	if (!str) return spec;

	if (str.peek_one() == '#') {
		str.take_one();
		spec.base_prefix = true;
	}

	if (str.peek_one() == '0') {
		str.take_one();
		spec.pad_type = FormatPadType::Zero;
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

}