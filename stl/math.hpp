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

// Represents a pair of values, which are
// semantically a point or a size.
template <class Type>
struct Vec2 {
	union {
		Type x, a, width;
	};
	union {
		Type y, b, height;
	};

	Vec2(Type x, Type y) : x(x), y(y) {}

	bool operator==(const Vec2& other) const {
		return x == other.x && y == other.y;
	}

	Vec2 operator+(const Vec2& other) const {
		return Vec2(x + other.x, y + other.y);
	}

	Vec2 operator+(const Type& offset) const {
		return Vec2(x + offset, y + offset);
	}

	Vec2 operator-(const Vec2& other) const {
		return Vec2(x - other.x, y - other.y);
	}

	Vec2 operator-(const Type& offset) const {
		return Vec2(x - offset, y - offset);
	}

	bool operator<(const Vec2& other) const {
		return x < other.x && y < other.y;
	}

	bool operator>(const Vec2& other) const {
		return x > other.x && y > other.y;
	}
};

// Represents a 2D rectangle at pos with size
template <class Type>
struct Rect {
	using Point = Vec2<Type>;
	Point pos;
	Point size;

	Rect(Type x, Type y, Type width, Type height) : pos(x, y), size(width, height) {}
	Rect(const Point& pos, const Point& size) : pos(pos), size(size) {}

	static Rect from_corners(const Point& top_left, const Point& bot_right) {
		return Rect(top_left, bot_right - top_left + 1);
	}

	bool contains(const Point& point) const {
		return point.x >= pos.x && point.x < pos.x + size.width &&
			point.y >= pos.y && point.y < pos.y + size.height;
	}

	bool intersects(const Rect& other) const {
		return math::max(left(), other.left()) < math::min(right(), other.right())
			&& math::max(top(), other.top()) < math::min(bottom(), other.bottom());
	}

	Point top_left() const { return pos; }
	Point top_right() const { return pos + Point(size.width - 1, 0); }
	Point bot_left() const { return pos + Point(0, size.height - 1); }
	Point bot_right() const { return pos + size - 1; }

	Type left() const { return pos.x; }
	Type right() const { return pos.x + size.width - 1; }

	Type top() const { return pos.y; }
	Type bottom() const { return pos.y + size.height - 1; }
};

}