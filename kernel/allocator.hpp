#pragma once

#include <stl/types.hpp>
#include "paging.hpp"

namespace kernel {

namespace alloc {

void init();

PhysicalAddress allocate_physical_page();
void free_physical_page(PhysicalAddress addr);

void* allocate_page();
void free_page(void* addr);

}

}