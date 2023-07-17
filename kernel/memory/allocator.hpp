#pragma once

#include <stl/types.hpp>
#include <kernel/memory/paging.hpp>

namespace kernel {

namespace alloc {

void init();

// "Allocates" a physical page of memory by marking it as used.
PhysicalAddress allocate_physical_page();

// Frees a physical page of memory by marking it as unused.
void free_physical_page(PhysicalAddress addr);


// Allocates a page of virtual memory. Equivalent to `allocate_pages(1)`.
// The page will be mapped as RWX.
void* allocate_page();

// Allocates "count" continuous pages.
// Each one should be freed individually (i think)
void* allocate_pages(usize count);

void free_page(void* addr);

}

}