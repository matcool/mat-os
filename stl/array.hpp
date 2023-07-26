#pragma once

#include "stl.hpp"
#include "types.hpp"

namespace STL_NS {

template <class Type, usize Size>
class Array {
	Type m_data[Size];

public:
	template <class... Args>
	requires(sizeof...(Args) <= Size && sizeof...(Args) != 0)
	Array(Args&&... args) : m_data{ args... } {}

	auto* data() { return m_data; }

	const auto* data() const { return m_data; }

	constexpr auto size() const { return Size; }

	auto begin() const { return data(); }

	auto begin() { return data(); }

	auto end() const { return data() + size(); }

	auto end() { return data() + size(); }

	auto& operator[](usize index) { return data()[index]; }

	const auto& operator[](usize index) const { return data()[index]; }
};

template <class Type, class... Args>
auto make_array(Args&&... args) {
	return Array<Type, sizeof...(Args)>(Type(args)...);
}

}