#pragma once

#include "span.hpp"
#include "stl.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "vector.hpp"

namespace STL_NS {

class StringView {
	const char* m_data = nullptr;
	usize m_size = 0;

public:
	StringView(const char* c_str) : m_data(c_str) {
		while (*c_str != 0) {
			++m_size;
			++c_str;
		}
	}

	StringView(const char* begin, const char* end) : m_data(begin) { m_size = end - begin; }

	const char* data() const { return m_data; }

	auto size() const { return m_size; }

	const char* begin() const { return data(); }

	const char* end() const { return data() + size(); }

	char operator[](usize index) const { return data()[index]; }

	bool operator==(const StringView& other) const {
		if (size() != other.size()) return false;
		for (usize i = 0; i < size(); ++i) {
			if (this->operator[](i) != other[i]) return false;
		}
		return true;
	}

	bool operator==(const char* c_str) const {
		for (char c : *this) {
			if (*c_str == 0 || *c_str != c) return false;
			++c_str;
		}
		return *c_str == 0;
	}

	operator bool() const { return size(); }

	StringView slice(usize start, usize end = -1) const {
		if (end == usize(-1)) end = size();
		return StringView(data() + start, data() + end);
	}

	usize find(char c) const {
		for (usize i = 0; i < size(); ++i) {
			if (this->operator[](i) == c) return i;
		}
		return usize(-1);
	}

	Pair<StringView, StringView> split_once(usize index) const {
		return { slice(0, index), slice(index + 1) };
	}

	// Peeks the first char from the string. useful for parsing
	char peek_one() const {
		// TODO: error?
		if (!size()) return 0;
		return (*this)[0];
	}

	// Takes the first char from the string, mutating it. useful for parsing
	char take_one() {
		if (!size()) return 0;
		const auto c = peek_one();
		m_data++;
		m_size--;
		return c;
	}

	Span<const char> span() const { return Span(m_data, m_size); }
};

inline StringView operator""_sv(const char* c_str, usize len) {
	return StringView(c_str, c_str + len);
}

// Returns whether a character is a decimal digit (0-9)
bool is_digit(char c);

// Returns whether a character is an ascii letter (a-zA-Z)
bool is_ascii_alpha(char c);

// Converts an ascii letter to uppercase, unchanged otherwise
char to_ascii_uppercase(char c);

// Converts an ascii letter to lowercase, unchanged otherwise
char to_ascii_lowercase(char c);

// A heap allocated string which can grow.
class String {
	Vector<char> m_data;

public:
	String(const char* c_str) : String(StringView(c_str)) {}

	String(StringView str) {
		m_data.reserve(str.size());
		m_data.concat(str.span());
	}

	String() = default;

	auto size() const { return m_data.size(); }

	auto data() { return m_data.data(); }

	auto data() const { return m_data.data(); }

	Span<char> span() { return Span(data(), size()); }

	Span<const char> span() const { return Span(data(), size()); }

	auto begin() { return data(); }

	auto begin() const { return data(); }

	auto end() { return data() + size(); }

	auto end() const { return data() + size(); }

	auto iter() { return Iterator(begin(), end()); }

	auto iter() const { return Iterator(begin(), end()); }

	operator StringView() const { return StringView(begin(), end()); }
};

}