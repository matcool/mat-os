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
	const u64 bit_mask = 1 << bit_index;
	if (value) {
		bitmap_array_ptr[array_index] |= bit_mask;
	} else {
		bitmap_array_ptr[array_index] &= ~bit_mask;
	}
}

void debug_print_memmap() {
	for (usize i = 0; i < memmap_request.response->entry_count; ++i) {
		auto* entry = memmap_request.response->entries[i];
		mat::StringView type = "?";
		switch (entry->type) {
			case LIMINE_MEMMAP_USABLE: type = "USABLE"; break;
			case LIMINE_MEMMAP_RESERVED: type = "RESERVED"; break;
			case LIMINE_MEMMAP_ACPI_RECLAIMABLE: type = "ACPI_RECLAIMABLE"; break;
			case LIMINE_MEMMAP_ACPI_NVS: type = "ACPI_NVS"; break;
			case LIMINE_MEMMAP_BAD_MEMORY: type = "BAD_MEMORY"; break;
			case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: type = "BOOTLOADER_RECLAIMABLE"; break;
			case LIMINE_MEMMAP_KERNEL_AND_MODULES: type = "KERNEL_AND_MODULES"; break;
			case LIMINE_MEMMAP_FRAMEBUFFER: type = "FRAMEBUFFER"; break;
		}
		kdbgln("[{}] - base: {:x} - length: {:x} - type: {}", i, entry->base, entry->length, type);
	}
}

void kernel::alloc::init() {
	if (!memmap_request.response || !hhdm_request.response)
		halt();

	debug_print_memmap();

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
		kdbgln("[PANIC] Couldn't find a memory region big enough for the bitmap array (size 0x{:x})", bitmap_array_size);
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

bool bitmap_get_index(usize index) {
	static constexpr auto entry_bits = sizeof(u64) * 8;
	const usize array_index = index / entry_bits;
	const u64 bit_index = index % entry_bits;
	const u64 bit_mask = 1 << bit_index;
	return bitmap_array_ptr[array_index] & bit_mask;
}

// both of these implementations are incredibly inefficient, but they should at least work

void* kernel::alloc::allocate_page() {
	usize page_index = 0;
	uptr page_address = 0;
	for (usize i = 0; i < memmap_request.response->entry_count; ++i) {
		auto* entry = memmap_request.response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			// start with size = PAGE_SIZE because we want to skip regions
			// smaller than the page size. so size ends up being the end of the page
			for (auto size = PAGE_SIZE; size <= entry->length; size += PAGE_SIZE) {
				if (!bitmap_get_index(page_index)) {
					page_address = entry->base + size - PAGE_SIZE;
					break;
				}
				page_index++;
			}
			if (page_address) break;
		}
	}
	
	if (!page_address) {
		kdbgln("[PANIC] Couldn't allocate a single page");
		halt();
	}

	bitmap_set_index(page_index, true);
	return reinterpret_cast<void*>(physical_to_virtual(page_address));
}

void kernel::alloc::free_page(void* pointer) {
	const auto address = reinterpret_cast<uptr>(pointer);
	if (address % PAGE_SIZE != 0) {
		kdbgln("[PANIC] Tried to free misaligned page ({:x})", address);
		halt();
	}

	const auto page_address = virtual_to_physical(address);
	usize page_index = 0;
	bool found = false;
	for (usize i = 0; i < memmap_request.response->entry_count; ++i) {
		auto* entry = memmap_request.response->entries[i];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			if (page_address >= entry->base && page_address < entry->base + entry->length) {
				page_index += (page_address - entry->base) / PAGE_SIZE;
				found = true;
				break;
			} else {
				page_index += entry->length / PAGE_SIZE;
			}
		}
	}

	if (!found) {
		kdbgln("[PANIC] Couldn't find page to free ({:x})", address);
		halt();
	}

	bitmap_set_index(page_index, false);
}