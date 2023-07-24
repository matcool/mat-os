#pragma once

#include "stl.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace STL_NS {

// Iterator impl namespace
namespace iterators {

struct IteratorEndTag {};

// i dont like this name
template <class Inner>
concept InnerIterator = requires (Inner value) {
	{ value.at_end() } -> types::is_same<bool>;
	value.next();
	value.value();
};

template <class It, class EndIt>
struct ItPair {
	It iterator;
	EndIt end_iterator;

	bool at_end() const { return iterator == end_iterator; }
	void next() { if (!at_end()) ++iterator; }
	decltype(auto) value() { return *iterator; }
};

template <InnerIterator Inner, class Func>
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

template <InnerIterator Inner>
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

template <InnerIterator Inner>
struct Enumerate {
	Inner inner;
	usize index = 0;

	using Type = decltype(inner.value());
	using ConvertedType = types::ternary<types::is_reference<Type>, types::remove_ref<Type>*, Type>;
	using TargetPair = Pair<usize, Type>;
	struct {
		usize index = 0;
		// Use this hacky union to store optional reference,
		// and also not require default construction of Type.
		union {
			bool nothing = false;
			ConvertedType value;
		};
	} hack;

	Enumerate(Inner value) : inner(value) {}
	
	bool at_end() const { return inner.at_end(); }
	void next() {
		inner.next();
		++index;
	}
	auto& value() {
		hack.index = index;
		if constexpr (types::is_reference<Type>) {
			hack.value = &inner.value();
		} else {
			hack.value = inner.value();
		}
		// Hehe
		// This is definitely UB, however a reference and a pointer are the same
		// in memory, so this should be fine
		return *reinterpret_cast<TargetPair*>(&hack);
	}
};

template <InnerIterator Inner, class Func>
struct Map {
	Inner inner;
	Func func;
	Map(Inner inner, Func func) : inner(inner), func(func) {}

	bool at_end() const { return inner.at_end(); }
	void next() { inner.next(); }
	decltype(auto) value() { return func(inner.value()); }
};

}

// Represents a pair of C++ iterators (begin and end), while offering
// many functional utils to apply on the iterator.
template <iterators::InnerIterator Inner>
class Iterator {
	Inner inner;
public:
	template <class T, class E>
	Iterator(T begin, E end) : inner(iterators::ItPair(begin, end)) {}
	Iterator(Inner inner) : inner(inner) {}

	auto& begin() { return *this; }
	auto end() { return iterators::IteratorEndTag(); }

	bool operator!=(iterators::IteratorEndTag) const { return !inner.at_end(); }
	void operator++() { inner.next(); }
	decltype(auto) operator*() { return inner.value(); }

	using ValueType = decltype(inner.value());

	// Filters element from the iterator according to `func`.
	// Only elements which `func(x) -> true` will be kept.
	template <class FilterFunc>
	requires requires (FilterFunc func, ValueType value) {
		{ func(value) } -> types::is_same<bool>;
	}
	auto filter(FilterFunc func) {
		return stl::Iterator(iterators::Filter(inner, func));
	}

	// Maps every element through a mapping function, yielding the result.
	template <class MapFunc>
	requires requires (MapFunc func, ValueType value) { func(value); }
	auto map(MapFunc func) {
		return stl::Iterator(iterators::Map(inner, func));
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

	// Enumerates the iterator by returning a pair of (index, value).
	auto enumerate() {
		return stl::Iterator(iterators::Enumerate(inner));
	}
};

template <class It, class EndIt>
Iterator(It, EndIt) -> Iterator<iterators::ItPair<It, EndIt>>;

namespace iterators {

namespace STL_NS_IMPL {

template <class Int>
struct Range {
	Int counter, end;
	Range(Int start, Int end) : counter(start), end(end) {}

	bool at_end() const { return counter >= end; }
	void next() { ++counter; }
	Int value() { return counter; }
};

}

// Returns an iterator that starts at `start`, and goes up
// to but not including `end`.
template <concepts::integral Int>
auto range(Int start, types::identity<Int> end) {
	return Iterator(STL_NS_IMPL::Range(start, end));
}

// Returns an iterator that goes from 0 to, but not inclduing, `end`.
template <concepts::integral Int>
auto range(Int end) {
	return Iterator(STL_NS_IMPL::Range(Int(0), end));
}

}

}