#pragma once

#include "stl.hpp"
#include "types.hpp"

STL_NS {

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

	const char* data() const { return m_data; }
	auto size() const { return m_size; }

	const char* begin() const { return m_data; }
	const char* end() const { return m_data + m_size; }
};

}