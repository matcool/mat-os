#pragma once

#include <stl/types.hpp>

inline u8 inb(u16 port) {
	u8 value;
	asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port) : "memory");
	return value;
}

inline void outb(u16 port, u8 value) {
	asm volatile("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

[[gnu::noreturn]] inline void halt() {
	asm ("cli");
	while (true) {
		asm ("hlt");
	}
}

inline u64 get_cr0() {
	u64 value;
	asm("movq %%cr0, %0" : "=r"(value));
	return value;
}

inline u64 get_cr2() {
	u64 value;
	asm("movq %%cr2, %0" : "=r"(value));
	return value;
}

inline u64 get_cr3() {
	u64 value;
	asm("movq %%cr3, %0" : "=r"(value));
	return value;
}

inline u64 get_cr4() {
	u64 value;
	asm("movq %%cr4, %0" : "=r"(value));
	return value;
}