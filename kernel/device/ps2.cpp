#include <kernel/device/pic.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>

void kernel::ps2::init() {
	// enable PS/2 keyboard
	pic::set_irq_mask(1, true);

	kdbgln("PS/2 devices initialized");
}