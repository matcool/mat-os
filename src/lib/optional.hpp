#pragma once
#include "stl.hpp"

template <class T>
class Optional {
	union {
		T m_value;
	};
	bool m_has_value;
public:
	constexpr Optional() : m_has_value(false) {}
	constexpr Optional(const T& value) : m_value(value), m_has_value(true) {}
	constexpr Optional(T&& value) : m_value(value), m_has_value(true) {}

	explicit constexpr operator bool() const { return m_has_value; }
	constexpr bool has_value() const { return m_has_value; }

	[[nodiscard]] constexpr T& value() { return m_value; }
	[[nodiscard]] constexpr const T& value() const { return m_value; }

	[[nodiscard]] constexpr T& operator*() { return m_value; }
	[[nodiscard]] constexpr const T& operator*() const { return m_value; }

	constexpr ~Optional() {
		if (m_has_value) {
			m_value.~T();
		}
	}
};
