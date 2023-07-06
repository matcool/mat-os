#pragma once

#include "serial.hpp"

// Kernel specific debug logging functions. Defaults to using serial output
#define kdbg(...) kernel::serial::fmt(__VA_ARGS__)
#define kdbgln(...) kernel::serial::fmtln(__VA_ARGS__)