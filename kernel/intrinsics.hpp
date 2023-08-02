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

[[gnu::noreturn]] inline void halt(bool disable_interrupts = true) {
	if (disable_interrupts) asm("cli");
	while (true) {
		asm("hlt");
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

// Disables interrupts
inline void cli() {
	asm volatile("cli");
}

// Enables interrupts
inline void sti() {
	asm volatile("sti");
}

inline uptr cpu_flags() {
	uptr value;
	asm volatile("pushf; pop %0" : "=rm"(value) : : "memory");
	return value;
}

namespace kernel {

// A simple guard that will disable interrupts.
struct DisableInterruptsGuard {
	bool ignore = false;

	DisableInterruptsGuard() {
		// if interrupts are already disabled, dont do anything
		ignore = !(cpu_flags() & 0b1000000000);
		cli();
	}

	~DisableInterruptsGuard() {
		if (!ignore) sti();
	}
};

}