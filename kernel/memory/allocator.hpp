#pragma once

#include <kernel/memory/paging.hpp>
#include <stl/types.hpp>

namespace kernel {

namespace alloc {

void init_physical_allocator();

// "Allocates" a physical page of memory by marking it as used.
[[nodiscard]] PhysicalAddress allocate_physical_page();

// Frees a physical page of memory by marking it as unused.
void free_physical_page(PhysicalAddress addr);

void init_virtual_allocator();

// Allocates a page of virtual memory. Equivalent to `allocate_pages(1)`.
// The page will be mapped as RWX.
[[nodiscard]] void* allocate_page();

// Allocates "count" continuous pages.
// Each one should be freed individually (i think)
[[nodiscard]] void* allocate_pages(usize count);

// Frees a single virtual page that was allocated via allocate_page(s).
void free_page(void* addr);

void init_heap_allocator();

// Allocates some amount of memory on the kernel heap.
// Returned pointer will be aligned to 16 bytes.
// In case of any failures, nullptr will be returned.
[[nodiscard]] void* heap_allocate(usize bytes);

// Frees some memory allocated by heap_allocate.
void heap_free(void* ptr);

// Initializes all allocators in the correct order.
inline void init() {
	init_physical_allocator();
	init_virtual_allocator();
	init_heap_allocator();
}

}

}