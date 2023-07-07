#include <limine/limine.h>
#include "allocator.hpp"
#include "log.hpp"
#include "intrinsics.hpp"
#include <stl/math.hpp>

static volatile limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = nullptr,
};

// limine maps physical memory -> virtual memory by just adding a higher half base
// this is constant except for when KASLR is on, so use this to get it
static volatile limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = nullptr,
};
uptr hhdm_base;

// Maps a physical address to virtual address, assuming limine's memory mapping
static uptr physical_to_virtual(uptr physical_address) {
	return physical_address + hhdm_base;
}

// Maps a virtual address to physical address, assuming limine's memory mapping
static uptr virtual_to_physical(uptr virtual_address) {
	return virtual_address - hhdm_base;
}

u64* bitmap_array_ptr = nullptr;

void bitmap_set_index(usize index, bool value) {
	static constexpr auto entry_bits = sizeof(u64) * 8;
	const usize array_index = index / entry_bits;
	const u64 bit_index = index % entry_bits;
	u64 bit_mask = 1 << bit_index;
	if (value)
		bit_mask = ~bit_mask;
	bitmap_array_ptr[array_index] &= bit_mask;
}

void kernel::alloc::init() {
	if (!memmap_request.response || !hhdm_request.response)
		halt();

	hhdm_base = hhdm_request.response->offset;

	usize usable_memory = 0;

	for (usize i = 0; i < memmap_request.response->entry_count; ++i) {
		auto* entry = memmap_request.response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			usable_memory += entry->length;
		}
	}

	kdbgln("In total, there seems to be {} MiB of usable memory", usable_memory / 1024 / 1024);

	const auto pages = usable_memory / PAGE_SIZE;
	// each byte holds 8 bits, each bit representing a page
	const auto bitmap_array_size = pages / 8;
	// the array itself also occupies some size, so we need to set those bits
	const auto bitmap_page_size = mat::math::div_ceil(bitmap_array_size, PAGE_SIZE);

	// iterate through the entries again to try to find space
	// keep track of how many pages we skipped
	usize skipped_pages = 0;
	for (usize i = 0; i < memmap_request.response->entry_count; ++i) {
		auto* entry = memmap_request.response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			if (entry->length >= bitmap_array_size) {
				bitmap_array_ptr = reinterpret_cast<u64*>(physical_to_virtual(entry->base));
				break;
			} else {
				skipped_pages += entry->length / PAGE_SIZE;
			}
		}
	}
	if (!bitmap_array_ptr) {
		kdbgln("[ERROR] Couldn't find a memory region big enough for the bitmap array (size 0x{:x})", bitmap_array_size);
		halt();
	}

	// zero out the array
	for (usize i = 0; i < (bitmap_array_size / sizeof(u64)); ++i) {
		bitmap_array_ptr[i] = 0;
	}

	// set the bits occupied by the bitmap array itself
	for (usize i = 0; i < bitmap_page_size; ++i) {
		bitmap_set_index(i + skipped_pages, true);
	}

	kdbgln("The bitmap array occupies {} KiB of space", bitmap_array_size / 1024);
}