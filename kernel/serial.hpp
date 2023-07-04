#pragma once

#include <stl/types.hpp>
#include <stl/string.hpp>

namespace kernel::serial {

void init();

void put_byte(u8 value);
void put_char(char value);
void put(mat::StringView str);

}