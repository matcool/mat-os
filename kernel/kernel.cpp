#include <stdint.h>
#include <stddef.h>
#include <limine.h>

// Halt and catch fire function.
static void hcf(void) {
    asm ("cli");
    while (true) {
        asm ("hlt");
    }
}

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

import serial;

extern "C" void _start() {
    serial::init();

    serial::put('h');
    serial::put('e');
    serial::put('l');
    serial::put('l');
    serial::put('o');
    serial::put('\n');

    if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    auto* framebuffer = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    for (size_t i = 0; i < framebuffer->height; i++) {
        uint32_t *fb_ptr = (uint32_t*)framebuffer->address;
        const size_t x = i;
        const size_t y = i;
        fb_ptr[y * (framebuffer->pitch / 4) + x] = 0xffffff;
    }

    // We're done, just hang...
    hcf();
}
