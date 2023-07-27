#include "units.hpp"
#include "array.hpp"

using namespace STL_NS;

void Formatter<units::Bytes>::format(format::Context ctx, units::Bytes bytes) {
	static constexpr usize threshold = 1200;
	static constexpr usize div = 1024;
	const auto units = make_array<StringView>("B"_sv, "KiB"_sv, "MiB"_sv, "GiB"_sv, "TiB"_sv);
	usize value = bytes;
	usize remainder = 0;
	usize i = 0;
	for (; i < units.size() - 1; ++i) {
		if (value < threshold) break;
		remainder = value % div;
		value /= div;
	}
	// get one decimal digit
	remainder = remainder * 10 / div;
	ctx.fmt_value(value);
	if (remainder) {
		ctx.put('.');
		ctx.put('0' + remainder);
	}
	ctx.put(' ');
	ctx.put(units[i]);
}