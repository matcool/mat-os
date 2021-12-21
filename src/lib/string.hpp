#pragma once
#include "stl.hpp"
#include "iterator.hpp"
#include "math.hpp"
#include "template-utils.hpp"
#include "hash.hpp"

class StringView : public Iterable<StringView> {
	const size_t m_size;
	const char* m_data;
public:
	StringView(const char* data, size_t size) : m_size(size), m_data(data) {}
	StringView(const char* data) : m_size(strlen(data)), m_data(data) {}

	size_t size() const { return m_size; }
	const char* data() const { return m_data; }

	char at(const size_t i) const { return m_data[i]; }
	char operator[](const size_t i) const { return m_data[i]; }

	bool operator==(const StringView& other) const {
		if (size() != other.size()) return false;
		for (size_t i = 0; i < m_size; ++i)
			if (at(i) != other.at(i)) return false;
		return true;
	}
};

inline StringView operator "" _sv(const char* data, size_t len) {
	return StringView(data, len);
}

template <>
struct Hash<StringView> {
	static HashType hash(const StringView& str) {
		return hash_combine(::hash(str.data()), ::hash(str.size()));
	}
};

// prob gonna change the name
template <class T = char, size_t inline_size = 15>
class BasicString : public Iterable<BasicString<T, inline_size>> {
	size_t m_size, m_capacity;
	union {
		T m_inline_data[inline_size + 1];
		T* m_data;
	};

	// it shouldnt ever go below it
	bool is_inline() { return m_capacity == inline_size; }
public:
	BasicString(const StringView& sv) : m_size(sv.size()), m_capacity(max(sv.size(), inline_size)) {
		if (!is_inline())
			m_data = new char[m_size + 1];
		const auto d = data();
		memcpy(d, sv.data(), m_size);
		d[m_size] = 0;
	}
	BasicString(const BasicString& string) : BasicString(string.sv()) {}
	BasicString(const char* str) : BasicString(StringView(str)) {}
	BasicString() : m_size(0), m_capacity(inline_size) {
		m_inline_data[0] = 0;
	}

	~BasicString() {
		if (!is_inline())
			delete[] m_data;
	}

	size_t size() const { return m_size; }
	size_t capacity() const { return m_capacity; }

	char* data() { return m_capacity <= inline_size ? m_inline_data : m_data; }
	const char* data() const { return m_capacity <= inline_size ? m_inline_data : m_data; }

	char& operator[](const size_t i) { return data()[i]; }
	char operator[](const size_t i) const { return data()[i]; }
	char& at(const size_t i) { return data()[i]; }
	char at(const size_t i) const { return data()[i]; }

	const StringView sv() const { return StringView(data(), m_size); }
	operator StringView() const { return sv(); }

	// hehe copy paste from Vector
	void reserve(const size_t size) {
		if (size > m_capacity) {
			auto new_location = operator new(sizeof(T) * (size + 1));
			memcpy(new_location, data(), sizeof(T) * m_size);
			if (!is_inline())
				delete[] m_data;
			m_data = reinterpret_cast<T*>(new_location);
			m_capacity = size;
		}
	}

	void push_back(const T value) {
		if (m_size >= m_capacity)
			reserve(m_capacity + 8);
		data()[m_size++] = value;
		data()[m_size] = 0;
	}

	void clear() {
		if (!is_inline())
			delete[] m_data;
		m_size = 0;
		m_capacity = inline_size;
		m_inline_data[0] = 0;
	}
};

using String = BasicString<>;
