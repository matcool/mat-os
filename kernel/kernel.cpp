#include <limine/limine.h>
#include <stl/types.hpp>
#include <stl/format.hpp>
#include "intrinsics.hpp"
#include "serial.hpp"
#include "idt.hpp"

static volatile limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
	.response = nullptr,
};

static volatile limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = nullptr,
};

using namespace kernel;

void triggering_interrupt() {
	volatile int x = 23 / 0;
}

extern "C" void _start() {
	serial::init();

	serial::put("Hello\n");

	serial::fmtln("Regular 50: {}, Hex 50: {:x}, Regular 10: {}", 50, 50, 10);

	if (!memmap_request.response)
		halt();

#if 0
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
		serial::fmtln("[{}] - base: {:x} - length: {:x} - type: {}", i, entry->base, entry->length, type);
	}
#endif

	idt::init();

	if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1) {
		halt();
	}

	auto* framebuffer = framebuffer_request.response->framebuffers[0];

	// Note: we assume the framebuffer model is RGB with 32-bit pixels.
	auto* const fb_ptr = (u32*)framebuffer->address;
	const auto stride = framebuffer->pitch / 4;
	for (usize y = 0; y < framebuffer->height; y++) {
		for (usize x = 0; x < framebuffer->width; x++) {
			u8 blue = ((x * y) >> 8) & 0xFF;
			u8 red = 0;
			u8 green = 0;
			u32 color = (red << 16) | (green << 8) | blue;
			fb_ptr[y * stride + x] = color;
		}
	}

	serial::fmtln("pre int 3");
	triggering_interrupt();
	serial::fmtln("post int 3");
}
