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
	auto d = (u8*)dst;
	auto s = (const u8*)src;
	while (len--) {
		*d++ = *s++;
	}
	return dst;
}

void memset(void* dst, u8 value, size_t len) {
	auto d = reinterpret_cast<u8*>(dst);
	while (len--)
		*d++ = value;
}