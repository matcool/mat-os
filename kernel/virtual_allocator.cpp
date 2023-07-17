#include "allocator.hpp"
#include "log.hpp"
#include "paging.hpp"

// start virtual allocations at 4 MiB, why not :-)
static constexpr uptr BASE_ADDRESS = 4 * 1024 * 1024;

// implement a simple bump allocator for now
usize allocated_pages = 0;

void* kernel::alloc::allocate_page() {
	return allocate_pages(1);
}

void* kernel::alloc::allocate_pages(usize count) {
	const auto addr = VirtualAddress(BASE_ADDRESS) + (allocated_pages * PAGE_SIZE);
	allocated_pages += count;

	for (usize i = 0; i < count; ++i) {
		const auto page = allocate_physical_page();
		paging::map_page(addr + i * PAGE_SIZE, page);
	}

	return addr.ptr();
}

void kernel::alloc::free_page(void*) {
	kdbgln("hehe sorry cant free");
}