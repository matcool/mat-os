#include <kernel/device/pic.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>

void kernel::ps2::init() {
	init_keyboard();

	kdbgln("PS/2 devices initialized");
}