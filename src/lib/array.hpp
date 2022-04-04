#pragma once
#include "stl.hpp"
#include "iterator.hpp"

template <class T, size_t N>
class Slice : public Iterable<Slice<T, N>> {
	T m_array[N];
public:
	// should i even implement these?
	Slice(const Slice&) = delete;
	Slice(Slice&&) = delete;

	Slice() {}

	Slice(const T& value) : m_array(value) {}

	template <class U>
	Slice(U&& init) : m_array(init) {}

	constexpr size_t size() const { return N; }
	const T* data() const { return m_array; }
	T* data() { return m_array; }

	const T& operator[](const size_t i) const { return m_array[i]; }
	const T& at(const size_t i) const { return m_array[i]; }
	T& operator[](const size_t i) { return m_array[i]; }
	T& at(const size_t i) { return m_array[i]; }
};

// a non owning view into an array of T
template <class T>
class View : public Iterable<View<T>> {
	T* m_data;
	size_t m_size;
public:
	View(T* const data, const size_t size) : m_data(data), m_size(size) {}

	size_t size() const { return m_size; }
	const T* data() const { return m_data; }
	T* data() { return m_data; }

	T& operator[](const size_t i) { return m_data[i]; }
	const T& operator[](const size_t i) const { return m_data[i]; }
};
