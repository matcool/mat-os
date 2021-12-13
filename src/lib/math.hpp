#pragma once
#include "stl.hpp"

template <class T, class... Args>
auto min(T a, Args... values) {
	if constexpr (sizeof...(Args) == 0) return a;
	else {
		const auto m = min(values...);
		return a < m ? a : m;
	}
}
