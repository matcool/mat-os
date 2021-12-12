#pragma once
#include "stl.hpp"

template <class T>
struct Iterator {
	size_t m_index;
	const T* m_container;

	Iterator& operator++() {
		++m_index;
		return *this;
	}

	bool operator!=(const Iterator& other) {
		return m_index != other.m_index;
	}

	auto& operator*() {
		return (*m_container)[m_index];
	}
};

// Inherit this to have basic forward iterators
// This assume T has size() and operator[]
template <class T>
struct Iterable {
	auto begin() { return Iterator<T> { 0, derived() }; }
	auto end() { return Iterator<T> { derived()->size(), derived() }; }

	auto begin() const { return Iterator<T> { 0, derived() }; }
	auto end() const { return Iterator<T> { derived()->size(), derived() }; }

private:
	inline const T* derived() const { return static_cast<const T*>(this); }
};
