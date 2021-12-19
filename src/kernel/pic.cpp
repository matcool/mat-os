#include "pic.hpp"

void pic_eoi(u8 irq) {
	if (irq >= 8)
		outb(PIC2_CMD, 0x20);
	outb(PIC1_CMD, 0x20);
}

void pic_remap(u8 offset) {
	// auto mask1 = inb(PIC1_DAT);
	// auto mask2 = inb(PIC2_DAT);

	outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();

	outb(PIC1_DAT, offset);
	io_wait();
	outb(PIC2_DAT, offset + 8);
	io_wait();

	outb(PIC1_DAT, 4);
	io_wait();
	outb(PIC2_DAT, 2);
	io_wait();

	outb(PIC1_DAT, ICW4_8086);
	io_wait();
	outb(PIC2_DAT, ICW4_8086);
	io_wait();

	// outb(PIC1_DAT, mask1);
	// outb(PIC2_DAT, mask2);
}

void pic_set_mask(u8 irq) {
	u16 port;

	if (irq < 8) {
		port = PIC1_DAT;
	} else {
		port = PIC2_DAT;
		irq -= 8;
	}
	outb(port, inb(port) | (1 << irq));
}

void pic_clear_mask(u8 irq) {
	u16 port;

	// TODO: not copy paste this mayb
	if (irq < 8) {
		port = PIC1_DAT;
	} else {
		port = PIC2_DAT;
		irq -= 8;
	}
	outb(port, inb(port) & ~(1 << irq));
}

// TODO: Spurious irqs

void pic_init() {
	pic_remap(0x20);

	// set mask for all irqs
	outb(PIC1_DAT, 0xFF);
	outb(PIC2_DAT, 0xFF);

	pic_clear_mask(1);
	pic_clear_mask(12);

	// saw on some random forum that you have to clear this to get mouse working
	// i have no idea why but its true
	pic_clear_mask(2);
}
