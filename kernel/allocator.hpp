#pragma once

#include <stl/types.hpp>

namespace kernel {

namespace alloc {

void init();

void* allocate_page();
void free_page(void* addr);

}

}