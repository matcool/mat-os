#pragma once
#include "stl.hpp"

class StringView {
	const size_t m_size;
	const char* m_data;
public:
	StringView(const char* data, size_t size) : m_size(size), m_data(data) {}
	StringView(const char* data) : m_size(strlen(data)), m_data(data) {}

	inline size_t size() const { return m_size; }
	inline const char* data() const { return m_data; }
};

inline StringView operator "" _sv(const char* data, size_t len) {
	return StringView(data, len);
}
