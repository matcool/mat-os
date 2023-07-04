#pragma once

#include "stl.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace STL_NS {

class StringView {
	char const* m_data = nullptr;
	usize m_size = 0;
public:
	StringView(const char* c_str) : m_data(c_str) {
		while (*c_str != 0) {
			++m_size;
			++c_str;
		}
	}
	StringView(const char* begin, const char* end) : m_data(begin) {
		m_size = end - begin;
	}

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
};

template <concepts::integral Int>
Int parse_int(StringView str) {
	bool negative = false;
	if (str[0] == '-') {
		negative = true;
		str = str.slice(1);
	}
	Int value;
	for (char c : str) {
		value *= 10;
		value += c - '0';
	}
	return negative ? -value : value;
}

}