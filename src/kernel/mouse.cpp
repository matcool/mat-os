#include "mouse.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "log.hpp"

INTERRUPT
void mouse_handler(InterruptFrame*) {
	serial("mouse ? {x}\n", inb(0x60));
	pic_eoi(12);
}

void mouse_wait_out() {
	u32 time_out = 100000;
	while (time_out && (inb(0x64) & 0b10) != 0) { --time_out; }
}
void mouse_wait_in() {
	u32 time_out = 100000;
	while (time_out && (inb(0x64) & 0b01) != 1) { --time_out; }
}

constexpr u8 COMMAND_PORT = 0x64;
constexpr u8 DATA_PORT = 0x60;

void ps2_write(u8 command) {
	mouse_wait_out();
	outb(COMMAND_PORT, command);
}

void ps2_write_data(u8 command) {
	mouse_wait_out();
	outb(DATA_PORT, command);
}

u8 ps2_read() {
	mouse_wait_in();
	return inb(DATA_PORT);
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
