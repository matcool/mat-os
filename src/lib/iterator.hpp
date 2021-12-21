#pragma once
#include "stl.hpp"

template <class T>
struct Iterator {
	size_t m_index;
	T* m_container;

	Iterator& operator++() {
		++m_index;
		return *this;
	}

	bool operator!=(const Iterator& other) {
		return m_index != other.m_index;
	}

	decltype(auto) operator*() const {
		return (*m_container)[m_index];
	}
};

// Inherit this to have basic forward iterators
// This assumes T has size() and operator[]
template <class T>
struct Iterable {
	auto begin() { return Iterator<T> { 0, derived() }; }
	auto end() { return Iterator<T> { derived()->size(), derived() }; }

	auto begin() const { return Iterator<const T> { 0, derived() }; }
	auto end() const { return Iterator<const T> { derived()->size(), derived() }; }

private:
	constexpr T* derived() { return static_cast<T*>(this); }
	constexpr const T* derived() const { return static_cast<const T*>(this); }
};
