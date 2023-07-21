#include <kernel/memory/allocator.hpp>
#include <kernel/memory/paging.hpp>
#include <kernel/log.hpp>

// start virtual allocations at 4 MiB, why not :-)
static constexpr uptr BASE_ADDRESS = 4 * 1024 * 1024;

// implement a simple bump allocator for now
usize allocated_pages = 0;

void kernel::alloc::init_virtual_allocator() {
	// nothing for now
}

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

void kernel::alloc::free_page(void* addr) {
	const auto value = reinterpret_cast<uptr>(addr);
	if (value < BASE_ADDRESS || !allocated_pages) {
		panic("Tried to free invalid address ({})", addr);
	}
	if (value == BASE_ADDRESS + (allocated_pages - 1) * PAGE_SIZE) {
		kdbgln("Freeing top most page");
		allocated_pages--;
		paging::unmap_page(VirtualAddress(value));
	} else {
		kdbgln("hehe sorry cant free");
	}
}