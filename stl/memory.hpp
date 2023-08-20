#pragma once

#include "stl.hpp"
#include "types.hpp"

namespace STL_NS {

// Fills the memory from ptr to ptr+bytes with value.
void memset(void* ptr, u8 value, usize bytes);

// Copies `size` bytes from src to dest.
void memcpy(void* dest, const void* src, usize size);

}

#if MAT_OS

// Placement operator new, does nothing to the pointer.
inline void* operator new(usize, void* ptr) noexcept {
	return ptr;
}

#else

#include <new>

#endif