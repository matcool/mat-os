#include <kernel/device/pic.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>

void send_pic_eoi(u8 irq) {
	if (irq >= 8) {
		outb(kernel::PIC2_COM_PORT, 0x20);
	}
	outb(kernel::PIC1_COM_PORT, 0x20);
}

void kernel::ps2::handle_keyboard() {
	const auto byte = inb(PS2_DATA_PORT);
	kdbg(" {:02x}", byte);
	
	send_pic_eoi(1);
}

void kernel::ps2::init() {
	// enable PS/2 keyboard
	pic::set_irq_mask(1, true);

	kdbgln("PS/2 devices initialized");
}