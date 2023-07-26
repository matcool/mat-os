#include <kernel/device/pic.hpp>
#include <kernel/device/pit.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/idt.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/memory/paging.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/serial.hpp>

using namespace kernel;

extern "C" void kernel_init() {
	serial::init();

	kdbgln("Booting up...");

	idt::init();

	paging::init();

	alloc::init();

	pic::init();
	ps2::init();
	pit::init();

	framebuffer::init();

	kdbgln("Finished initialization");

	framebuffer::loop();
}
