#pragma once
#include "common.hpp"

constexpr u8 PS2_COMMAND_PORT = 0x64;
constexpr u8 PS2_DATA_PORT = 0x60;

inline void ps2_wait_out() {
	u32 time_out = 100000;
	while (time_out && (inb(0x64) & 0b10) != 0) { --time_out; }
}

inline void ps2_wait_in() {
	u32 time_out = 100000;
	while (time_out && (inb(0x64) & 0b01) != 1) { --time_out; }
}

inline void ps2_write(u8 command) {
	ps2_wait_out();
	outb(PS2_COMMAND_PORT, command);
}

inline void ps2_write_data(u8 command) {
	ps2_wait_out();
	outb(PS2_DATA_PORT, command);
}

inline u8 ps2_read() {
	ps2_wait_in();
	return inb(PS2_DATA_PORT);
}
