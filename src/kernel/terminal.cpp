#include "terminal.hpp"
#include "screen.hpp"
#include <lib/math.hpp>
#include <lib/vector.hpp>

constexpr auto FONT_WIDTH = 7;
constexpr auto FONT_HEIGHT = 9;
constexpr auto PIXEL_SIZE = 2;
constexpr auto FONT_PIXEL_WIDTH = FONT_WIDTH * PIXEL_SIZE;
constexpr auto FONT_PIXEL_HEIGHT = FONT_HEIGHT * PIXEL_SIZE;

u32 cursor_x = 0;
u32 cursor_y = 0;
u32 width, height;
Vector<char> buffer;

void terminal_init() {
	auto& screen = Screen::get();
	width = screen.width / FONT_PIXEL_WIDTH;
	height = screen.height / FONT_PIXEL_HEIGHT;
	// TODO: either Vector::fill or ctor with size and initial value param
	buffer.reserve(width * height);
	for (size_t i = 0; i < width * height; ++i)
		buffer.push_back(' ');
}

#include "terminal-font.hpp"

void terminal_put_entry_at(char c, size_t x, size_t y) {
	buffer[y * width + x] = c;
}

void terminal_scroll_down() {
	--cursor_y;
	for (u32 y = 0; y < height - 1; ++y)
		memcpy(&buffer[y * width], &buffer[(y + 1) * width], width);
	for (u32 x = 0; x < width; ++x)
		terminal_put_entry_at(' ', x, height - 1);
}

void terminal_put_char(char c) {
	if (c != '\n')
		terminal_put_entry_at(c, cursor_x, cursor_y);
	const auto& screen = Screen::get();
	if (c == '\n' || ++cursor_x == screen.width / FONT_PIXEL_WIDTH) {
		cursor_x = 0;
		if (++cursor_y == screen.height / FONT_PIXEL_HEIGHT) {
			terminal_scroll_down();
		}
	}
}

void terminal_delete_char() {
	if (cursor_x == 0) {
		--cursor_y;
		cursor_x = width;
	}
	--cursor_x;
	terminal_put_entry_at(' ', cursor_x, cursor_y);
}

void terminal_draw_char(char c, u32 x, u32 y, u32 color) {
	const auto letter = terminal_font[u8(c)];
	auto& screen = Screen::get();
	for (u32 font_j = 0; font_j < FONT_HEIGHT; ++font_j) {
		const auto row = letter[font_j];
		for (u32 font_i = 0; font_i < FONT_WIDTH; ++font_i) {
			if (row & (1 << font_i)) {
				for (u8 offset_y = 0; offset_y < PIXEL_SIZE; ++offset_y) {
					for (u8 offset_x = 0; offset_x < PIXEL_SIZE; ++offset_x) {
						const auto pixel_y = y * FONT_PIXEL_HEIGHT + font_j * PIXEL_SIZE + offset_y;
						const auto pixel_x = x * FONT_PIXEL_WIDTH + font_i * PIXEL_SIZE + offset_x;
						screen.set_pixel(pixel_x, pixel_y, color);
					}
				}
			}
		}
	}
}

void terminal_draw() {
	for (u32 y = 0; y < height; ++y) {
		for (u32 x = 0; x < width; ++x) {
			terminal_draw_char(buffer[y * width + x], x, y, 0xFFFFFFFF);
		}
	}
}
