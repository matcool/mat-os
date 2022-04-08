#pragma once
#include "stl.hpp"
#include "iterator.hpp"
#include "template-utils.hpp"
#include "utils.hpp"

template <class T>
class Vector : public Iterable<Vector<T>> {
	size_t m_size, m_capacity;
	T* m_data;
public:
	Vector(size_t capacity) : m_size(0), m_capacity(capacity),
		m_data(capacity ? reinterpret_cast<T*>(operator new(sizeof(T) * m_capacity)) : nullptr) {}

	Vector() : Vector(0) {}

	Vector(const Vector& other) : m_size(other.m_size), m_capacity(m_size), m_data(nullptr) {
		if (m_size) {
			m_data = reinterpret_cast<T*>(operator new(sizeof(T) * m_capacity));
			copy(other.begin(), other.end(), m_data);
		}
	}

	~Vector() {
		for (size_t i = 0; i < m_size; ++i)
			destroy(m_data[i]);
		// note its not delete[]
		// because i avoid new[] as it always initializes every element
		delete m_data;
	}

	Vector& operator=(const Vector& other) {
		// TODO: maybe operator= on each element instead?
		clear(); // doesnt deallocate
		reserve(other.m_size);
		m_size = other.m_size;
		for (size_t i = 0; i < m_size; ++i) {
			new (&m_data[i]) T(other[i]);
		}
		return *this;
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

	void pop_back() {
		if (m_size)
			destroy(m_data[--m_size]);
	}

	bool empty() const { return !m_size; }
	T& back() { return m_data[m_size - 1]; }
};
