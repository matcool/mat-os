#include "mouse.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "serial.hpp"
#include "log.hpp"
#include "ps2.hpp"
#include "screen.hpp"

bool left_down = false;
bool right_down = false;
bool middle_down = false;

u32 mouse_x = 0;
u32 mouse_y = 0;
u32 prev_mouse_x, prev_mouse_y;

u8 cycle_counter = 0;
u8 data_bytes[3];

INTERRUPT
void mouse_handler(InterruptFrame*) {
	const auto status = inb(PS2_COMMAND_PORT);
	// buffer is full, ignore
	if ((status & 1) == 0) {
		pic_eoi(12);
		return;
	}
	data_bytes[cycle_counter++] = inb(PS2_DATA_PORT);
	if (cycle_counter == 3) {
		cycle_counter = 0;
		u8 data = data_bytes[0];
		i16 x_mov = data_bytes[1];
		i16 y_mov = data_bytes[2];
		if ((data & 0b11000000) == 0) {
			if (data & 0b010000)
				x_mov -= 256;
			if (data & 0b100000)
				y_mov -= 256;
			middle_down = !!(data & 0b100);
			right_down = !!(data & 0b010);
			left_down = !!(data & 0b001);
			prev_mouse_x = mouse_x;
			prev_mouse_y = mouse_y;
			if (i32(mouse_x) < -x_mov)
				mouse_x = 0;
			else
				mouse_x += x_mov;
			if (i32(mouse_y) < y_mov)
				mouse_y = 0;
			else
				mouse_y -= y_mov;
			auto& screen = Screen::get();
			mouse_x = max(min(mouse_x, screen.width - 1), 0u);
			mouse_y = max(min(mouse_y, screen.height - 1), 0u);
			// screen.redraw();
			mouse_draw();
		}
	}
	pic_eoi(12);
}

void mouse_send_command(u8 command) {
	ps2_write(0xD4);
	ps2_write_data(command);
	const auto ans = ps2_read();
	if (ans != 0xFA)
		log("Mouse did not respond with ACK ({x})", ans);
}

// TODO: this doesnt work
void mouse_set_rate(u8 rate) {
	mouse_send_command(0xF3);
	ps2_write_data(rate);
	const auto ans = ps2_read();
	if (ans != 0xFA)
		log("Mouse did not respond with ACK ({x}) while trying to set rate to {}", ans, rate);
}

u8 mouse_get_id() {
	mouse_send_command(0xF2);
	return ps2_read();
}

void mouse_init() {
	ps2_write(0xA8);

	ps2_write(0x20);
	const auto config_byte = ps2_read() | 2;
	ps2_write(0x60);
	ps2_write_data(config_byte & 0xDF);

	mouse_send_command(0xF6);

	// mouse_set_rate(200);
	// mouse_set_rate(100);
	// mouse_set_rate(80);
	// serial("this should be 3: {}\n", mouse_get_id());

	mouse_send_command(0xF4);

	idt_get_table()[0x20 + 12] = IDTEntry(isr_wrapper<&mouse_handler>, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);

	log("Mouse initialized");
}

const u32 mouse_sprite[13] = {
	0b000000000000000001,
	0b000000000000000101,
	0b000000000000011101,
	0b000000000001111101,
	0b000000000111111101,
	0b000000011111111101,
	0b000001111111111101,
	0b000111111111111101,
	0b010111111111111101,
	0b000001011111111101,
	0b000000011101011101,
	0b000001110100000101,
	0b000000010000000000
};

void mouse_draw() {
	auto& screen = Screen::get();
	// this can def be done with just one set of double for loops,
	// however i am too lazy rn
	for (u32 j = 0; j < 13; ++j) {
		auto row = mouse_sprite[j];
		for (u32 i = 0; i < 9; ++i) {
			if (row & 0b11) {
				const auto index = (prev_mouse_y + j) * screen.width + prev_mouse_x + i;
				screen.buffer_a[index] = screen.buffer_b[index];
			}
			row >>= 2;
		}
	}
	for (u32 j = 0; j < 13; ++j) {
		auto row = mouse_sprite[j];
		for (u32 i = 0; i < 9; ++i) {
			const u8 b = row & 0b11;
			if (b) {
				const u32 color = b == 1 ? 0xFF000000 : 0xFFFFFFFF;
				const auto pixel_y = mouse_y + j;
				const auto pixel_x = mouse_x + i;
				screen.buffer_a[pixel_y * screen.width + pixel_x] = color;
			}
			row >>= 2;
		}
	}
}
