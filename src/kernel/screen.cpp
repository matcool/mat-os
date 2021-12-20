#include "screen.hpp"
#include "terminal.hpp"
#include "mouse.hpp"

void Screen::redraw() {
	for (size_t i = 0; i < width * height; ++i) {
		raw_buffer[i] = 0xFF223344;
	}
	terminal_draw();
	mouse_draw();
}
