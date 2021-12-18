#include "terminal.hpp"

static struct {
	u32 x, y;
	u32 width, height;
	u32* buffer;
} state;

void terminal_init(u32* addr, u32 width, u32 height) {
	state.x = 0;
	state.y = 0;
	state.width = width;
	state.height = height;
	state.buffer = addr;
}

constexpr auto FONT_SIZE = 8;
constexpr auto PIXEL_SIZE = 2;
constexpr auto FONT_WIDTH = FONT_SIZE * PIXEL_SIZE;

#include "font8x8_basic.h"

static const auto letters = font8x8_basic;

void terminal_put_entry_at(char c, u32 color, size_t x, size_t y) {
	const auto letter = letters[c];
	for (u32 font_j = 0; font_j < FONT_SIZE; ++font_j) {
		const auto row = letter[font_j];
		for (u32 font_i = 0; font_i < FONT_SIZE; ++font_i) {
			// if (row & (1 << (FONT_SIZE - font_i - 1))) {
			if (row & (1 << font_i)) {
				for (u8 offset_y = 0; offset_y < PIXEL_SIZE; ++offset_y) {
					for (u8 offset_x = 0; offset_x < PIXEL_SIZE; ++offset_x) {
						state.buffer[(y * FONT_WIDTH + font_j * PIXEL_SIZE + offset_y) * state.width
									+ x * FONT_WIDTH + font_i * PIXEL_SIZE + offset_x] = color;

					}
				}
			}
		}
	}
}

void terminal_put_char(char c) {
	if (c != '\n')
		terminal_put_entry_at(c, 0xFFFFFFFF, state.x, state.y);
	if (c == '\n' || ++state.x == state.width / FONT_WIDTH) {
		state.x = 0;
		if (++state.y == state.height / FONT_WIDTH) {
			state.y = state.height / FONT_WIDTH - 1;
		}
	}
}
