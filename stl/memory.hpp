#pragma once

#include "stl.hpp"
#include "types.hpp"

namespace STL_NS {

// Fills the memory from ptr to ptr+bytes with value.
void memset(void* ptr, u8 value, usize bytes);

}

// Placement operator new, does nothing to the pointer.
inline void* operator new(usize, void* ptr) {
	return ptr;
}