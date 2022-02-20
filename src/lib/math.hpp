#pragma once
#include "stl.hpp"
#include "template-utils.hpp"

template <class T, class... Args>
constexpr auto min(T a, Args&&... values) {
	if constexpr (sizeof...(Args) == 0) return a;
	else {
		const auto m = min(forward<decltype(values)>(values)...);
		return a < m ? a : m;
	}
}

template <class T, class... Args>
constexpr auto max(T a, Args&&... values) {
	if constexpr (sizeof...(Args) == 0) return a;
	else {
		const auto m = max(forward<decltype(values)>(values)...);
		return a > m ? a : m;
	}
}

template <number T>
constexpr T abs(T value) {
	if constexpr (is_floating_point<T> || is_signed<T>) {
		if (value < 0)
			return -value;
		return value;
	} else {
		return value;
	}
}

template <class T>
struct Vec2 {
	union {
		T x, a, width;
	};
	union {
		T y, b, height;
	};

	Vec2(T x, T y) : x(x), y(y) {}

	bool operator==(const Vec2& other) const {
		return x == other.x && y == other.y;
	}

	Vec2 operator+(const Vec2& other) const {
		return { x + other.x, y + other.y };
	}
};
