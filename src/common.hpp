#pragma once
#include <stddef.h>
#include <stdint.h>

#define PACKED __attribute__((packed))

// TODO: maybe split these types into another file

using u8 = uint8_t;
using i8 = int8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using u64 = uint64_t;
using i64 = int64_t;

using uptr = uintptr_t;

inline void outb(u16 port, u8 value) {
	asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline u8 inb(u16 port) {
	u8 ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

inline void io_wait() {
	outb(0x80, 0);
}

// mat lib c

size_t strlen(const char* str);

void* memcpy(void* dst, const void* src, size_t len);
void memset(void* dst, u8 val, size_t len);

void* malloc(size_t size);
void free(void* addr);