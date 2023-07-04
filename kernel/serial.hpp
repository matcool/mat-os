#pragma once

#include <stl/types.hpp>
#include <stl/string.hpp>

namespace kernel::serial {

void init();

void put(u8 value);
void put(mat::StringView str);

}