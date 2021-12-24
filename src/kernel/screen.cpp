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
	// clear();
	// terminal_draw();

	swap();
	// mouse_draw();
}

void Window::update_screen(u32 a, u32 b, u32 width, u32 height) {
	auto& screen = Screen::get();
	for (u32 y = b; y < height + b; ++y) {
		for (u32 x = a; x < width + a; ++x) {
			screen.set_pixel(m_position.x + x, m_position.y + y, m_buffer[y * m_size.width + x]);
		}
	}
	// TODO: not
	screen.swap();
}

void Window::update_entire_thing_lol() {
	auto& screen = Screen::get();
	if (m_prev_position != m_position) {
		// TODO: intersection with current pos to not set the same pixel twice
		for (u32 y = 0; y < m_size.height; ++y) {
			for (u32 x = 0; x < m_size.width; ++x) {
				screen.set_pixel(m_prev_position.x + x, m_prev_position.y + y, 0xFF223344);
			}
		}
	}
	update_screen(0, 0, m_size.width, m_size.height);
	m_prev_position = m_position;
}
