#include "mouse.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "log.hpp"
#include "ps2.hpp"
#include "kernel.hpp"

bool left_down = false;
bool right_down = false;
bool middle_down = false;

i32 mouse_x = 0;
i32 mouse_y = 0;

INTERRUPT
void mouse_handler(InterruptFrame*) {
	auto data = inb(PS2_DATA_PORT);
	i16 x_mov = inb(PS2_DATA_PORT);
	i16 y_mov = inb(PS2_DATA_PORT);
	if ((data & 0b11000000) == 0) {
		if (data & 0b010000)
			x_mov -= 256;
		if (data & 0b100000)
			y_mov -= 256;
		middle_down = !!(data & 0b100);
		right_down = !!(data & 0b010);
		left_down = !!(data & 0b001);
		mouse_x += x_mov;
		mouse_y -= y_mov;
		serial("mouse {},{}\n", mouse_x, mouse_y);
		mouse_x = max(min(mouse_x, Screen::width - 1), 0);
		mouse_y = max(min(mouse_y, Screen::height - 1), 0);
		Screen::raw[mouse_y * Screen::width + mouse_x] = 0xFF000000;
		Screen::raw[mouse_y * Screen::width + mouse_x + 1] = 0xFF000000;
		Screen::raw[(mouse_y + 1) * Screen::width + mouse_x] = 0xFF000000;
		Screen::raw[(mouse_y + 1) * Screen::width + mouse_x + 1] = 0xFF000000;
	}
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
