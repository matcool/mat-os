#include <limine/limine.h>
#include <stl/types.hpp>
#include <stl/format.hpp>
#include "intrinsics.hpp"
#include "serial.hpp"
#include "idt.hpp"
#include "log.hpp"

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

void trigger_interrupt() {
	volatile int* x = nullptr;
	*x = 10;
	// volatile int y = *x;
}

using namespace kernel;

extern "C" void _start() {
	serial::init();

	serial::put("Hello\n");

	kdbgln("Regular 50: {}, Hex 50: {:x}, Regular 10: {}", 50, 50, 10);

	if (!memmap_request.response)
		halt();

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

	kdbgln("Finished");
	trigger_interrupt();
	kdbgln("yup");
	halt();
}
