#include "screen.hpp"
#include "terminal.hpp"
#include "mouse.hpp"

void Screen::init(u32 width, u32 height, u32* pixels) {
	this->width = width;
	this->height = height;
	buffer_a = pixels;
	buffer_b = static_cast<u32*>(malloc(sizeof(u32) * width * height));
}

void Screen::clear() {
	for (size_t i = 0; i < width * height; ++i)
		buffer_b[i] = 0xFF223344;
}

void Screen::swap() {
	for (size_t i = 0; i < width * height; ++i)
		buffer_a[i] = buffer_b[i];
}

void Screen::redraw() {
	clear();
	terminal_draw();

	swap();
	mouse_draw();
}
