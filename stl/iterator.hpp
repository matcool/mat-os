#pragma once

#include "stl.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace STL_NS {

// Iterator impl namespace
namespace iterators {

template <class It, class EndIt>
struct ItPair {
	It iterator;
	EndIt end_iterator;

	bool at_end() const { return iterator == end_iterator; }
	void next() { if (!at_end()) ++iterator; }
	decltype(auto) value() { return *iterator; }
};

template <class Inner, class Func>
struct Filter {
	Inner inner;
	Func func;
	
	Filter(Inner value, Func func) : inner(value), func(func) {
		skip();
	}
	
	bool at_end() const { return inner.at_end(); }
	decltype(auto) value() { return inner.value(); }
	void next() {
		inner.next();
		skip();
	}

	void skip() {
		while (!inner.at_end() && !func(inner.value())) {
			inner.next();
		}
	}
};

template <class Inner>
struct Take {
	Inner inner;
	usize counter = 0;
	usize max;
	
	Take(Inner value, usize max) : inner(value), max(max) {}
	
	decltype(auto) value() { return inner.value(); }
	bool at_end() const {
		return counter >= max || inner.at_end();
	}
	void next() {
		inner.next();
		++counter;
	}
};

struct IteratorEndTag {};

}

template <class Pair>
class Iterator {
	Pair inner;
public:
	template <class T, class E>
	Iterator(T begin, E end) : inner(iterators::ItPair(begin, end)) {}

	Iterator(Pair inner) : inner(inner) {}

	auto& begin() { return *this; }
	auto end() { return iterators::IteratorEndTag(); }

	bool operator!=(iterators::IteratorEndTag) const { return !inner.at_end(); }
	void operator++() { inner.next(); }
	decltype(auto) operator*() { return inner.value(); }

	// Filters element from the iterator according to `func`.
	// Only elements which `func(x) -> true` will be kept.
	template <class FilterFunc>
	auto filter(FilterFunc func) {
		return stl::Iterator(iterators::Filter(inner, func));
	}

	// Skips the first n elements from the iterator.
	auto skip(usize n) {
		auto copy = inner;
		for (usize i = 0; i < n; ++i) {
			copy.next();
		}
		return Iterator(copy);
	}

	// Stops the iterator after `n` iterations.
	auto take(usize n) {
		return stl::Iterator(iterators::Take(inner, n));
	}
};

template <class It, class EndIt>
Iterator(It, EndIt) -> Iterator<iterators::ItPair<It, EndIt>>;

}