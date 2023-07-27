#pragma once

#include "format.hpp"
#include "stl.hpp"

namespace STL_NS {

namespace units {

struct Bytes {
	usize value;

	constexpr Bytes(usize value) : value(value) {}

	constexpr operator usize() const { return value; }
};

}

template <>
struct Formatter<units::Bytes> {
	static void format(format::Context ctx, units::Bytes bytes);
};

}