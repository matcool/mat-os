#include <kernel/device/pit.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/tasks/scheduler.hpp>
#include <kernel/window/manager.hpp>
#include <limine/limine.h>
#include <stl/memory.hpp>
#include <stl/types.hpp>

static volatile limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
	.response = nullptr,
};

Canvas& kernel::framebuffer::get_framebuffer() {
	static Canvas instance(nullptr, 0, 0);
	return instance;
}

static Canvas& get_screen_framebuffer() {
	static Canvas instance(nullptr, 0, 0);
	return instance;
}

void kernel::framebuffer::init() {
	if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1)
		panic("None or invalid response for framebuffer request");

	auto* framebuffer = framebuffer_request.response->framebuffers[0];

	// we assume that the framebuffer uses 32 bit pixels!
	auto* const pixels = reinterpret_cast<u32*>(framebuffer->address);
	const auto stride = framebuffer->pitch / 4;

	auto& fb = get_screen_framebuffer() =
		Canvas(pixels, framebuffer->width, framebuffer->height, stride);

	const auto width = fb.width();
	const auto height = fb.height();
	auto* const new_pixels = (u32*)(operator new(width* height * sizeof(u32)));
	memset(new_pixels, 0, width * height * sizeof(u32));
	get_framebuffer() = Canvas(new_pixels, width, height, width);

	window::WindowManager::get().init();
	window::WindowManager::get().paint();

	kdbgln("Framebuffer initialized");
}

void kernel::framebuffer::loop() {
	while (true) {
#if DEBUG_DRAW_RECTS
		if (window::WindowManager::get().changed()) {
			get_screen_framebuffer().paste(get_framebuffer(), 0, 0);
			window::WindowManager::get().draw_debug(&get_screen_framebuffer());
		}
#else
		get_screen_framebuffer().paste(get_framebuffer(), 0, 0);
#endif
		sleep(8);
		kernel::tasks::yield_thread();
	}
}