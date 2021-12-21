#pragma once
#include "stl.hpp"
#include "template-utils.hpp"

template <class T>
struct Hash {
	static_assert(always_false_t<T>, "Unimplemented hash for type T");
};

// should this be hash_type? hash_t? idk
// should this even exist at all
using HashType = size_t;

template <integral T>
struct Hash<T> {
	static HashType hash(const T& value) {
		return static_cast<unsigned_of<T>>(value);
	}
};

// shorthand version that avoids having to type Hash<T>::hash(value)
// while also removing const and refs
template <class T>
HashType hash(T&& value) {
	return Hash<remove_cv<remove_ref<T>>>::hash(forward<T>(value));
}

template <class T>
requires is_pointer<T>
struct Hash<T> {
	static HashType hash(const T& value) {
		return reinterpret_cast<uptr>(value);
	}
};

namespace {
	// taken from boost::hash_combine
	template <class T, class... Ts>
	inline void _hash_combine(HashType& seed, const T& value, Ts&&... rest) {
		seed ^= Hash<T>::hash(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		(_hash_combine(seed, forward<decltype(rest)>(rest)), ...);
	}
}

template <class... Args>
HashType hash_combine(Args&&... args) {
	HashType seed = 0;
	_hash_combine(seed, args...);
	return seed;
}
