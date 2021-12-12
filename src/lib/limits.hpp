#pragma once
#include "stl.hpp"
#include "template-utils.hpp"

template <class T>
struct NumberLimit {
	static_assert(always_false_t<T>, "Unsupported type");
};

template <>
struct NumberLimit<i32> {
	static constexpr auto max = 2147483647;
	static constexpr auto min = -2147483648;
};

template <>
struct NumberLimit<u32> {
	static constexpr auto max = 4294967295u;
	static constexpr auto min = 0;
};

template <>
struct NumberLimit<i64> {
	static constexpr auto max = 9223372036854775807;
	static constexpr auto min = i64(-9223372036854775808);
};

template <>
struct NumberLimit<u64> {
	static constexpr auto max = 18446744073709551615u;
	static constexpr auto min = 0;
};