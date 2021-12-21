#pragma once
#include "stl.hpp"
#include "hash.hpp"

// TODO: a better name for this header Lol

template <class K, class V>
class HashMap;

template <class T>
class HashSet {
	// maybe make this packed?
	struct Element {
		T value;
		Element* next;
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
		size_t m_index = 0;
		Element* m_element = nullptr;
		HashSet<T>* m_set;

		HashIterator(HashSet<T>* set) : m_set(set) {
			until_next();
		}
		HashIterator(size_t index, Element* element, HashSet<T>* set)
			: m_index(index), m_element(element), m_set(set) {}

		void until_next() {
			while (!m_element && m_index < m_set->m_capacity) {
				m_element = m_set->m_elements[++m_index];
			}
			if (m_index == m_set->m_capacity) m_element = nullptr;
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

		bool operator!=(const HashIterator& other) const {
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
			slot = new Element { value, nullptr };
			return slot->value;
		} else {
			auto el = slot;
			while (el->next) {
				if (el->value == value) return el->value;
				el = el->next;
			}
			if (el->value == value) return el->value;
			++m_size;
			el->next = new Element { value, nullptr };
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

	auto begin() { return HashIterator(this); }
	auto end() { return HashIterator(m_capacity, nullptr, this); }
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

