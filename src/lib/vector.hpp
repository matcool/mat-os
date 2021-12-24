#pragma once
#include "stl.hpp"
#include "iterator.hpp"
#include "template-utils.hpp"

template <class T>
class Vector : public Iterable<Vector<T>> {
	size_t m_size, m_capacity;
	T* m_data;
public:
	Vector(size_t capacity = 3) : m_size(0), m_capacity(capacity),
		m_data(capacity ? reinterpret_cast<T*>(operator new(sizeof(T) * m_capacity)) : nullptr) {}

	~Vector() {
		for (size_t i = 0; i < m_size; ++i)
			destroy(m_data[i]);
		// note its not delete[]
		delete m_data;
	}

	size_t size() const { return m_size; }
	size_t capacity() const { return m_capacity; }

	const T* data() const { return m_data; }
	T* data() { return m_data; }

	T& operator[](const size_t i) { return m_data[i]; }
	T& at(const size_t i) { return m_data[i]; }
	const T& operator[](const size_t i) const { return m_data[i]; }
	const T& at(const size_t i) const { return m_data[i]; }

	void reserve(const size_t size) {
		if (size > m_capacity) {
			auto new_location = operator new(sizeof(T) * size);
			memcpy(new_location, (void*)m_data, sizeof(T) * m_size);
			delete m_data;
			m_data = reinterpret_cast<T*>(new_location);
			m_capacity = size;
		}
	}

	void push_back(const T& value) {
		if (m_size >= m_capacity)
			reserve(m_capacity + 8);
		new (&m_data[m_size++]) T(value);
	}

	void push_back(T&& value) {
		if (m_size >= m_capacity)
			reserve(m_capacity + 8);
		new (&m_data[m_size++]) T(forward<T>(value));
	}

	void clear() {
		for (size_t i = 0; i < m_size; ++i)
			destroy(m_data[i]);
		m_size = 0;
	}
};
