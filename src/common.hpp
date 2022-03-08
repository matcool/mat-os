#pragma once
#include "types.hpp"

#define PACKED __attribute__((packed))

inline void outb(u16 port, u8 value) {
	asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline u8 inb(u16 port) {
	u8 ret;
	asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

inline void cli() {
	asm volatile("cli");
}

inline void sti() {
	asm volatile("sti");
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

void* operator new(size_t, void* ptr);
