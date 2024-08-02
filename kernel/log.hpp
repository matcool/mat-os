#pragma once

#include <kernel/intrinsics.hpp>
#include <kernel/serial.hpp>

// Kernel specific debug logging functions. Defaults to using serial output
#define kdbg kernel::serial::fmt
#define kdbgln kernel::serial::fmtln

#define panic(...)                                              \
	do {                                                        \
		kdbg("[PANIC] at {}:{}\n[PANIC] ", __FILE__, __LINE__); \
		kdbgln(__VA_ARGS__);                                    \
		halt();                                                 \
	} while (0)
