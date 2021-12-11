#include "terminal.hpp"
 
uint8_t vga_entry_color(vga_color fg, vga_color bg)  {
	return fg | bg << 4;
}
 
uint16_t vga_entry(unsigned char uc, uint8_t color)  {
	return (uint16_t) uc | (uint16_t) color << 8;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_init() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_scroll_down() {
	--terminal_row;
	for (size_t y = 1; y < VGA_HEIGHT; y++) {
		memcpy(&terminal_buffer[(y - 1) * VGA_WIDTH], &terminal_buffer[y * VGA_WIDTH], VGA_WIDTH * sizeof(*terminal_buffer));
	}
}
 
void terminal_set_color(uint8_t color) {
	terminal_color = color;
}
 
void terminal_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_put_char(char c) {
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
		return;
	}
	terminal_put_entry_at(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}
 
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_put_char(data[i]);
}
 
void terminal_write_string(const char* data) {
	terminal_write(data, strlen(data));
}