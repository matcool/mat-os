#pragma once
#include "stl.hpp"

template <class A, class B>
struct Pair {
	A first;
	B second;
};

template <class A, class B>
bool operator==(const Pair<A, B>& a, const Pair<A, B>& b) {
	return a.first == b.first && a.second == b.second;
}

template <class Input, class Output>
void copy(Input begin, Input end, Output out) {
	for (; begin != end; ++begin, ++out)
		*out = *begin;
}

template <class T>
requires requires (T& a) { a.swap(a); }
void swap(T& a, T& b) {
	a.swap(b);
}

template <class T>
void swap(T& a, T& b) {
	const T copy = a;
	a = b;
	b = copy;
}
