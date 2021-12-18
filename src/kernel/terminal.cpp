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

constexpr auto FONT_WIDTH = 7;
constexpr auto FONT_HEIGHT = 9;
constexpr auto PIXEL_SIZE = 2;
constexpr auto FONT_PIXEL_WIDTH = FONT_WIDTH * PIXEL_SIZE;
constexpr auto FONT_PIXEL_HEIGHT = FONT_HEIGHT * PIXEL_SIZE;

#include "terminal-font.hpp"

void terminal_put_entry_at(char c, u32 color, size_t x, size_t y) {
	const auto letter = terminal_font[c];
	for (u32 font_j = 0; font_j < FONT_HEIGHT; ++font_j) {
		const auto row = letter[font_j];
		for (u32 font_i = 0; font_i < FONT_WIDTH; ++font_i) {
			if (row & (1 << font_i)) {
				for (u8 offset_y = 0; offset_y < PIXEL_SIZE; ++offset_y) {
					for (u8 offset_x = 0; offset_x < PIXEL_SIZE; ++offset_x) {
						const auto pixel_y = y * FONT_PIXEL_HEIGHT + font_j * PIXEL_SIZE + offset_y;
						const auto pixel_x = x * FONT_PIXEL_WIDTH + font_i * PIXEL_SIZE + offset_x;
						state.buffer[pixel_y * state.width + pixel_x] = color;
					}
				}
			}
		}
	}
}

void terminal_put_char(char c) {
	if (c != '\n')
		terminal_put_entry_at(c, 0xFFFFFFFF, state.x, state.y);
	if (c == '\n' || ++state.x == state.width / FONT_PIXEL_WIDTH) {
		state.x = 0;
		if (++state.y == state.height / FONT_PIXEL_HEIGHT) {
			state.y = state.height / FONT_PIXEL_HEIGHT - 1;
		}
	}
}
