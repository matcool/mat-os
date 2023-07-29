#pragma once

#include <stl/types.hpp>

namespace kernel::ps2 {

static constexpr u16 PS2_DATA_PORT = 0x60;
static constexpr u16 PS2_COM_PORT = 0x64;

// Writes a command to the PS/2 command port
void write(u8 command);

// Writes some data to the PS/2 data port
void write_data(u8 command);

// Reads data from teh PS/2 data port
u8 read();

void init();

void init_keyboard();

void handle_keyboard();

void init_mouse();

void handle_mouse();

enum class KeyKind {
	Other,
	Printable,

	Escape,
	Enter,
	Backspace,
	CapsLock,
	Left,
	Right,
	Up,
	Down,

	LeftCtrl,
	RightCtrl,
	LeftShift,
	RightShift,
	LeftAlt,
	RightAlt
};

struct Key {
	KeyKind kind = KeyKind::Other;
	char ch = 0;
};

}