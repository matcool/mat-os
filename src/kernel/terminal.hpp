#pragma once
#include "common.hpp"
#include <lib/string.hpp>
#include <lib/format.hpp>

// Hardware text mode color constants.
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

uint8_t vga_entry_color(vga_color fg, vga_color bg);

constexpr size_t VGA_WIDTH = 80;
constexpr size_t VGA_HEIGHT = 25;

void terminal_init();
void terminal_scroll_down();
void terminal_set_color(uint8_t color);
void terminal_put_entry_at(char c, uint8_t color, size_t x, size_t y);
void terminal_put_char(char);
void terminal_write(const char* data, size_t len);
void terminal_write_string(const char* string);
void terminal_delete_char();

template <class... Args>
void terminal(const StringView& string, Args&&... args) {
	format_to([](char c) { terminal_put_char(c); }, string, args...);
}
