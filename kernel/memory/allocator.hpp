#pragma once

#include <stl/types.hpp>
#include <kernel/memory/paging.hpp>

namespace kernel {

namespace alloc {

void init();

PhysicalAddress allocate_physical_page();
void free_physical_page(PhysicalAddress addr);

void* allocate_page();
void free_page(void* addr);

// Allocates "count" continuous pages.
// Each one should be freed individually (i think)
void* allocate_pages(usize count);

}

}