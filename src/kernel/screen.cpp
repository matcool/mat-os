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

void Screen::refresh() {
	for (const auto& rect : m_update_rects) {
		for (u32 y = rect.pos.y; y < rect.size.height + rect.pos.y; ++y) {
			for (u32 x = rect.pos.x; x < rect.size.width + rect.pos.x; ++x) {
				const auto i = y * width + x;
				buffer_a[i] = buffer_b[i];
			}
		}
	}
	// serial("update rects cleared\n");
	m_update_rects.clear();
	for (auto& window : m_windows) {
		window->m_has_refreshed_move = false;
	}
}

void Window::update_screen(const Vec2<u32> pos, const Vec2<u32> size) {
	auto& screen = Screen::get();
	for (u32 y = pos.y; y < size.height + pos.y; ++y) {
		for (u32 x = pos.x; x < size.width + pos.x; ++x) {
			screen.set_pixel(m_position.x + x, m_position.y + y, m_buffer[y * m_size.width + x]);
		}
	}
	// TODO: not
	if (screen.m_update_rects.size() < 50)
		screen.m_update_rects.push_back(Rect { m_position + pos, size });
}

void Window::update_entire_thing_lol() {
	if (m_prev_position != m_position) {
		auto& screen = Screen::get();
		// TODO: intersection with current pos to not set the same pixel twice
		for (u32 y = 0; y < m_size.height; ++y) {
			for (u32 x = 0; x < m_size.width; ++x) {
				screen.set_pixel(m_prev_position.x + x, m_prev_position.y + y, 0xFF223344);
			}
		}
		screen.m_update_rects.push_back(Rect { m_prev_position, m_size });
		update_screen({ 0, 0 }, m_size);
		m_prev_position = m_position;
	}
}
