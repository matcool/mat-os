#pragma once
#include "stl.hpp"

class StringView {
	const size_t m_size;
	const char* m_data;
public:
	StringView(const char* data, size_t size) : m_size(size), m_data(data) {}
	StringView(const char* data) : m_size(strlen(data)), m_data(data) {}

	size_t size() const { return m_size; }
	const char* data() const { return m_data; }

	char at(const size_t i) const { return m_data[i]; }
	char operator[](const size_t i) const { return m_data[i]; }
};

inline StringView operator "" _sv(const char* data, size_t len) {
	return StringView(data, len);
}
