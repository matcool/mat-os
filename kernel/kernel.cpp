#include <limine/limine.h>
#include <stl/types.hpp>
#include <stl/format.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/serial.hpp>
#include <kernel/idt.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/memory/paging.hpp>
#include <kernel/device/pic.hpp>
#include <kernel/device/ps2.hpp>

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

	paging::init();

	alloc::init();

	void* page = alloc::allocate_page();
	void* page2 = alloc::allocate_pages(1);
	reinterpret_cast<u64*>(page2)[0] = 0x69696969;
	reinterpret_cast<u64*>(page2)[2] = 0x4201337;

	kdbgln("addr of page: {}", page);
	kdbgln("another page: {}", page2);

	// should fail
	alloc::free_page(page);
	// should work since its the top most page
	alloc::free_page(page2);
	// crash
	// reinterpret_cast<u64*>(page2)[0] = 0x69696969;

	page = alloc::allocate_page();
	kdbgln("new page: {}", page);
	void* page3 = alloc::allocate_page();
	kdbgln("page 3: {}", page3);

	// this has a 2mb page!
	paging::explore_addr((uptr)framebuffer_request.response);
	// the two virtual addresses should be different, but both point to the same
	// physical address
	paging::explore_addr((uptr)&_start);

	pic::init();
	ps2::init();

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

	kdbgln("Finished, halting");
	// halt without disabling interrupts
	while (true) {
		asm volatile("hlt");
	}
}
