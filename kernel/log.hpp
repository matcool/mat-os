#pragma once

#include <kernel/serial.hpp>
#include <kernel/intrinsics.hpp>

// Kernel specific debug logging functions. Defaults to using serial output
#define kdbg(...) kernel::serial::fmt(__VA_ARGS__)
#define kdbgln(...) kernel::serial::fmtln(__VA_ARGS__)

#define panic(...) do { kdbg("[PANIC] at {}:{}\n[PANIC] ", __FILE__, __LINE__); kdbgln(__VA_ARGS__); halt(); } while (0)