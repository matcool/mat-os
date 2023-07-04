#include <limine/limine.h>
#include <stl/types.hpp>
#include <stl/format.hpp>
#include "intrinsics.hpp"
#include "serial.hpp"

static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
	.response = nullptr,
};

using namespace kernel;

extern "C" void _start() {
	serial::init();

	serial::put("Hello\n");

	mat::format_to(&serial::put_char, "Hello {} world {}|{}|{}|{}\n", true, false, 20, 0, -13);

	if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1) {
		halt();
	}

	auto* framebuffer = framebuffer_request.response->framebuffers[0];

	// Note: we assume the framebuffer model is RGB with 32-bit pixels.
	for (usize i = 0; i < framebuffer->height; i++) {
		u32 *fb_ptr = (u32*)framebuffer->address;
		const usize x = i;
		const usize y = i;
		fb_ptr[y * (framebuffer->pitch / 4) + x] = 0xffffff;
	}

	halt();
}
