#include "mouse.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "log.hpp"
#include "ps2.hpp"

INTERRUPT
void mouse_handler(InterruptFrame*) {
	serial("mouse ? {x}\n", inb(PS2_DATA_PORT));
	pic_eoi(12);
}

void mouse_send_command(u8 command) {
	ps2_write(0xD4);
	ps2_write_data(command);
	if (ps2_read() != 0xFA)
		log("Mouse did not respond with ACK");
}

void mouse_init() {
	ps2_write(0xA8);

	ps2_write(0x20);
	const auto config_byte = ps2_read() | 2;
	serial("config byte is {x}\n", config_byte);
	ps2_write(0x60);
	ps2_write_data(config_byte & 0xDF);
	mouse_send_command(0xF6);
	mouse_send_command(0xF4);

	idt_get_table()[0x20 + 12] = IDTEntry(isr_wrapper<&mouse_handler>, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);

	log("Mouse initialized");
}
