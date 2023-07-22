#pragma once

#include "stl.hpp"
#include "utils.hpp"

namespace STL_NS::math {

// Divides two integers and return the ceiling
template <concepts::integral Int>
constexpr Int div_ceil(Int a, Int b) {
	return a / b + !!(a % b);
}

// Returns an integer with the first "n" bits set to 1.
template <concepts::integral Int>
constexpr Int bit_mask(Int n) {
	return (Int(1) << n) - 1;
}

// Sets a specific bit at "idx" to a value.
template <concepts::integral Int>
[[nodiscard]] constexpr Int set_bit(Int target, u64 idx, bool value) {
	const Int mask = Int(1) << idx;
	return (target & ~mask) | (value << idx);
}

// Gets a specific bit at "idx".
template <concepts::integral Int>
constexpr bool get_bit(Int value, u64 idx) {
	const Int mask = Int(1) << idx;
	return value & mask;
}

// Returns the smaller of two values, according to their < operator.
template <class ComparableType>
[[nodiscard]] constexpr auto min(ComparableType a, ComparableType b) {
	return a < b ? a : b;
}

// Returns the bigger of two values, according to their > operator.
template <class ComparableType>
[[nodiscard]] constexpr auto max(ComparableType a, ComparableType b) {
	return a > b ? a : b;
}

// Clamps a value to be lower <= value <= higher
template <class Type>
[[nodiscard]] constexpr auto clamp(Type value, types::identity<Type> lower, types::identity<Type> higher) {
	return value < lower ? lower : value > higher ? higher : value;
}

}