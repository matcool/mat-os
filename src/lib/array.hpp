#pragma once
#include "stl.hpp"
#include "iterator.hpp"

template <class T, size_t N>
class Slice {
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

	auto begin() { return Iterator<Slice> { .m_index = 0, .m_container = this }; }
	auto end() { return Iterator<Slice> { .m_index = N, .m_container = this }; }

};
