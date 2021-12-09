#include "common.hpp"

// this is sort of my libc

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void* memcpy(void* dst, const void* src, size_t len) {
	// TODO: optimize this
	auto d = (uint8_t*)dst;
	auto s = (const uint8_t*)src;
	while (len--) {
		*d++ = *s++;
	}
	return dst;
}