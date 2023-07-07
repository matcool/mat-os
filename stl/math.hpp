#pragma once

#include "stl.hpp"
#include "utils.hpp"

namespace STL_NS::math {

// Divides two integers and return the ceiling
template <concepts::integral Int>
Int div_ceil(Int a, Int b) {
	return a / b + !!(a % b);
}

}