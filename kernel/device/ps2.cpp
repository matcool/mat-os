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
	kdbgln("ps2 data port has {:02x}", inb(PS2_DATA_PORT));
	
	send_pic_eoi(1);
}