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

// TODO: have this in a separate file

template <class K, class V>
class HashMap;

template <class T>
class HashSet {
	// maybe make this packed?
	struct Element {
		Element* next;
		T value;
	};
	size_t m_size;
	size_t m_capacity;
	Element** m_elements;

	size_t index_for_value(const T& value) const {
		return hash(value) % m_capacity;
	}

	template <class K, class V>
	friend class HashMap;

	struct HashIterator {
		size_t m_index;
		Element* m_element;
		const HashSet<T>* m_set;

		void until_next() {
			while (!m_element && m_index < m_set->m_capacity) {
				m_element = m_set->m_elements[++m_index];
			}
		}

		// this would be const if i didnt use it for the
		// hashmap, in most other cases you dont want to modify
		// this since it would change the hash
		HashIterator& operator++() {
			if (m_element)
				m_element = m_element->next;
			until_next();
			return *this;
		}

		bool operator!=(const HashIterator& other) {
			return m_index != other.m_index || m_element != other.m_element;
		}

		T& operator*() const {
			return m_element->value;
		}
	};
public:
	HashSet() : m_size(0), m_capacity(16) {
		m_elements = new Element*[m_capacity];
	}
	~HashSet() {
		for (size_t i = 0; i < m_capacity; ++i) {
			auto slot = m_elements[i];
			while (slot) {
				const auto next = slot->next;
				delete slot;
				slot = next;
			}
		}
		delete[] m_elements;
	}

	T& insert(const T& value) {
		auto& slot = m_elements[index_for_value(value)];
		if (slot == nullptr) {
			++m_size;
			slot = new Element { nullptr, value };
			return slot->value;
		} else {
			auto el = slot;
			while (el->next) {
				if (el->value == value) return el->value;
				el = el->next;
			}
			if (el->value == value) return el->value;
			++m_size;
			el->next = new Element { nullptr, value };
			return el->next->value;
		}
	}

	bool contains(const T& value) const {
		auto el = m_elements[index_for_value(value)];
		while (el) {
			if (el->value == value) return true;
			el = el->next;
		}
		return false;
	}

	size_t size() const { return m_size; }

	auto begin() { return HashIterator { 0, m_elements[0], this }; }
	auto end() { return HashIterator { m_capacity, nullptr, this }; }
};

namespace {
	template <class K, class V>
	struct HashKeyValue {
		K key;
		V value;

		bool operator==(const HashKeyValue& other) {
			return key == other.key;
		}
	};
}

template <class K, class V>
struct Hash<HashKeyValue<K, V>> {
	static HashType hash(const auto& value) {
		return ::hash(value.key);
	}
};

template <class K, class V>
class HashMap {
	HashSet<HashKeyValue<K, V>> m_container;
public:
	HashMap() {}

	V& operator[](const K& key) {
		return m_container.insert({ key, V() }).value;
	}

	size_t size() const { return m_container.size(); }

	auto begin() { return m_container.begin(); }
	auto end() { return m_container.end(); }
};

