#pragma once

#include "iterator.hpp"
#include "stl.hpp"
#include "types.hpp"

namespace STL_NS {

// A span represents a pointer to some array with dynamic size.
template <class Type>
class Span {
	Type* m_data;
	usize m_size;

public:
	Span(Type* data, usize size) : m_data(data), m_size(size) {}

	Span() : m_data(nullptr), m_size(0) {}

	Type* data() { return m_data; }

	const Type* data() const { return m_data; }

	auto size() const { return m_size; }

	auto begin() { return data(); }

	auto begin() const { return data(); }

	auto end() { return data() + size(); }

	auto end() const { return data() + size(); }

	auto iter() { return Iterator(begin(), end()); }

	auto iter() const { return Iterator(begin(), end()); }

	Type& operator[](usize index) { return data()[index]; }

	const Type& operator[](usize index) const { return data()[index]; }

	// Implicit conversion to span of const elements
	operator Span<const Type>() const { return Span<const Type>(data(), size()); }

	bool empty() const { return !m_size; }

	operator bool() const { return !empty(); }

	Span sub(usize start, usize count = USIZE_MAX) {
		if (count == USIZE_MAX) count = size() - start;
		return Span(data() + start, count);
	}
};

}