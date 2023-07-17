#pragma once

#include "stl.hpp"
#include "types.hpp"

namespace STL_NS {

// Fills the memory from ptr to ptr+bytes with value.
void memset(void* ptr, u8 value, usize bytes);

}