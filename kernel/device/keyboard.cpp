#include <kernel/device/ps2.hpp>
#include <kernel/device/pic.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>

void kernel::ps2::handle_keyboard() {
	const auto byte = inb(PS2_DATA_PORT);
	kdbg(" {:02x}", byte);
	
	pic::send_eoi(1);
}