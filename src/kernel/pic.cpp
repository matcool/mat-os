#include "pic.hpp"

void pic_eoi(u8 irq) {
	if (irq >= 8)
		outb(PIC2_CMD, 0x20);
	outb(PIC1_CMD, 0x20);
}

void pic_remap(u8 offset) {
	auto mask1 = inb(PIC1_DAT);
	auto mask2 = inb(PIC2_DAT);

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
 
	outb(PIC1_DAT, mask1);
	outb(PIC2_DAT, mask2);
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

u16 _pic_get_irq_reg(u8 value) {
	outb(PIC1_CMD, value);
	outb(PIC2_CMD, value);
	return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

u16 pic_get_irr() {
	return _pic_get_irq_reg(PIC_READ_IRR);
}

u16 pic_get_isr() {
	return _pic_get_irq_reg(PIC_READ_ISR);
}

// TODO: Spurious irqs

void pic_init() {
    pic_remap(0x20);
}