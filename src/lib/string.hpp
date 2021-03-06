#pragma once
#include "stl.hpp"
#include "iterator.hpp"
#include "math.hpp"
#include "template-utils.hpp"
#include "hash.hpp"
#include "limits.hpp"
#include "utils.hpp"

// TODO: maybe make this not inline?
inline bool is_whitespace(const char ch) {
	return ch == ' '
	|| ch == '\n'
	|| ch == '\t'
	|| ch == '\r'
	|| ch == '\f' // line feed
	|| ch == '\v'; // vertical tab
}

class StringView : public Iterable<StringView> {
	size_t m_size;
	const char* m_data;
public:
	constexpr StringView(const char* data, size_t size) : m_size(size), m_data(data) {}
	StringView(const char* data) : m_size(strlen(data)), m_data(data) {}
	StringView() : m_size(0), m_data(nullptr) {}

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

	explicit operator bool() const { return m_size; }

	StringView sub(size_t start, size_t end = NumberLimit<size_t>::max) const {
		if (end == NumberLimit<size_t>::max) end = m_size;
		return StringView(m_data + start, end - start);
	}

	bool starts_with(const StringView& str) const {
		if (m_size < str.size()) return false;
		return sub(0, str.size()) == str;
	}

	size_t find(const char c) const {
		for (size_t i = 0; i < m_size; ++i)
			if (m_data[i] == c) return i;
		return -1;
	}

	Pair<StringView, StringView> split_once(const char c) const {
		const auto pos = find(c);
		if (pos == size_t(-1)) return { *this, StringView("", 0) };
		return { sub(0, pos), sub(pos + 1) };
	}

	auto split_iter(const char split) const {
		struct SplitIterator {
			StringView cur, rest;
			char split;
			void next(const StringView& str) {
				const auto [a, b] = str.split_once(split);
				cur = a;
				rest = b;
			}
			SplitIterator(const StringView str, const char split) : split(split) {
				next(str);
			}
			auto& operator++() {
				next(rest);
				return *this;
			}
			auto operator*() {
				return cur;
			}
			bool operator!=(int) {
				return cur || rest;
			}
		};
		struct SplitWrapper {
			StringView str;
			char split;
			auto begin() { return SplitIterator(str, split); }
			auto end() { return 0; }
		};
		return SplitWrapper { *this, split };
	}

	StringView trim_left() const {
		for (size_t i = 0; i < m_size; ++i) {
			if (!is_whitespace(m_data[i])) return sub(i);
		}
		// if all of it is whitespace then return an empty string
		return StringView("", 0);
	}

	StringView trim_right() const {
		for (size_t i = m_size; i; --i) {
			if (!is_whitespace(m_data[i - 1])) return sub(0, i);
		}
		// if all of it is whitespace then return an empty string
		return StringView("", 0);
	}

	auto trim() const { return trim_left().trim_right(); }
};

inline constexpr StringView operator "" _sv(const char* data, size_t len) {
	return StringView(data, len);
}

template <>
struct Hash<StringView> {
	static HashType hash(const StringView& str) {
		return hash_combine(str.data(), str.size());
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

	BasicString(BasicString&& other) : m_size(other.m_size), m_capacity(other.m_capacity) {
		if (other.is_inline())
			copy(other.m_inline_data, other.m_inline_data + other.m_size + 1, m_inline_data);
		else
			m_data = other.m_data;
		other.m_size = 0;
		other.m_capacity = inline_size;
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

	bool operator==(const StringView& other) const {
		return sv() == other;
	}

	// hehe copy paste from Vector
	void reserve(const size_t size) {
		if (size > m_capacity) {
			auto new_location = new T[size + 1];
			memcpy(new_location, data(), sizeof(T) * m_size);
			if (!is_inline())
				delete[] m_data;
			m_data = new_location;
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

	void pop() {
		if (m_size)
			data()[--m_size] = 0;
	}
};

using String = BasicString<>;
