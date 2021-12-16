#pragma once
#include "stl.hpp"
#include "template-utils.hpp"

template <class T, class... Args>
auto min(T a, Args&&... values) {
	if constexpr (sizeof...(Args) == 0) return a;
	else {
		const auto m = min(forward<decltype(values)>(values)...);
		return a < m ? a : m;
	}
}

template <class T, class... Args>
auto max(T a, Args&&... values) {
	if constexpr (sizeof...(Args) == 0) return a;
	else {
		const auto m = max(forward<decltype(values)>(values)...);
		return a > m ? a : m;
	}
}
