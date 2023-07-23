#pragma once

#include "stl.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "memory.hpp"
#include "span.hpp"

namespace STL_NS {

// Stores a dynamically sized linear array of elements.
template <class Type>
class Vector {
	Type* m_data = nullptr;
	usize m_capacity = 0;
	usize m_size = 0;

	static Type* allocate_buffer(usize size) {
		return static_cast<Type*>(operator new(sizeof(Type) * size));
	}
public:
	Vector() : Vector(0) {}
	Vector(usize capacity) {
		m_data = allocate_buffer(capacity);
		m_capacity = capacity;
	}

	auto size() const { return m_size; }
	auto capacity() const { return m_capacity; }
	auto* data() { return m_data; }
	const auto* data() const { return m_data; }

	auto begin() const { return data(); }
	auto begin() { return data(); }
	auto end() const { return data() + size(); }
	auto end() { return data() + size(); }

	Type& operator[](usize index) { return data()[index]; }
	const Type& operator[](usize index) const { return data()[index]; }

	// Resizes the vector to a given capacity
	void resize(usize new_capacity) {
		if (new_capacity < capacity()) return;
		auto* new_buffer = allocate_buffer(new_capacity);
		// TODO: trivially copyable should use memcpy
		for (usize i = 0; i < size(); ++i) {
			new (&new_buffer[i]) Type(move(m_data[i]));
			m_data[i].~Type();
		}
		delete m_data;
		m_data = new_buffer;
		m_capacity = new_capacity;
	}

	// Puts a new element at the end of the vector
	void push(Type value) {
		grow_if_needed();
		new (&m_data[m_size++]) Type(move(value));
	}

	// Pops the last element of the vector
	void pop() {
		// error?
		if (size() == 0) return;
		data()[--m_size].~Type();
	}

	// Removes an element at an index
	void remove(usize index) {
		for (usize i = index; i < size() - 1; ++i) {
			m_data[i] = m_data[i + 1];
		}
		m_data[size() - 1].~Type();
		--m_size;
	}

	Span<Type> span() { return Span(data(), size()); }
	Span<const Type> span() const { return Span(data(), size()); }

	// Concats multiple elements to the end of the vector
	void concat(Span<const Type> elements) {
		for (auto& el : elements) {
			push(el);
		}
	}

protected:
	// Resizes the vector if there is not enough space for a *single* new item
	void grow_if_needed() {
		if (capacity() == 0) {
			resize(4);
		} else if (capacity() == size()) {
			resize(capacity() * 2);
		}
	}
};

};