#include "keyboard.hpp"
#include "idt.hpp"
#include "serial.hpp"
#include "terminal.hpp"
#include "pic.hpp"

INTERRUPT
void keyboard_interrupt(InterruptFrame*) {
	auto scancode = inb(0x60);
	terminal_put_char(scancode);
	// serial_put_hex(scancode); serial_put_char(' ');
	pic_eoi(1);
}

void keyboard_init() {
	// outb(0x64, 0xF0);
	// outb(0x60, 2);
	// io_wait();
	// if (inb(0x60) == 0xFA) {
	// 	serial_put_string("successfully set scancode set to 2\n");
		idt_get_table()[0x20 + 1] = IDTEntry(isr_wrapper<&keyboard_interrupt>, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);
	// }

}
