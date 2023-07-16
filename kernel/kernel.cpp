#include <limine/limine.h>
#include <stl/types.hpp>
#include <stl/format.hpp>
#include "intrinsics.hpp"
#include "serial.hpp"
#include "idt.hpp"
#include "log.hpp"
#include "allocator.hpp"

static volatile limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
	.response = nullptr,
};

using namespace kernel;

extern "C" void _start() {
	serial::init();

	serial::put("Hello\n");

	kdbgln("Regular 50: {}, Hex -50: {:#x}, Regular 10: {}", 50, -50, 10);
	kdbgln("does it work? {} {}", true, "i guess");

	idt::init();

	alloc::init();

	void* page = alloc::allocate_page();
	void* page2 = alloc::allocate_page();

	kdbgln("addr of page: {}", page);
	kdbgln("another page: {}", page2);

	alloc::free_page(page);

	page = alloc::allocate_page();
	kdbgln("new page: {}", page);
	void* page3 = alloc::allocate_page();
	kdbgln("page 3: {}", page3);


	if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1) {
		halt();
	}

	auto* framebuffer = framebuffer_request.response->framebuffers[0];

	// Note: we assume the framebuffer model is RGB with 32-bit pixels.
	auto* const fb_ptr = (u32*)framebuffer->address;
	const auto stride = framebuffer->pitch / 4;
	for (usize y = 0; y < framebuffer->height; y++) {
		for (usize x = 0; x < framebuffer->width; x++) {
			u8 blue = 255 - (x * (y + 400) >> 8 & 0xFF) / 2;
			u8 red = blue / 2;
			u8 green = blue / 2;
			u32 color = (red << 16) | (green << 8) | blue;
			fb_ptr[y * stride + x] = color;
		}
	}

	kdbgln("CR0: {:#032b}", get_cr0());
	kdbgln("CR3: {:#032b}", get_cr3());
	kdbgln("CR4: {:#032b}", get_cr4());

	kdbgln("Finished, halting");
	halt();
}
