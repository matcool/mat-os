#include <stl/math.hpp>
#include <kernel/screen/terminal.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/screen/terminal_font.hpp>
#include <kernel/log.hpp>

using mat::math::get_bit;

const u32 width = 7;
const u32 height = 10;
u32 columns = 0;
u32 column = 0;
u32 row = 0;

static u32 darken_color(u32 color) {
	const auto r = color >> 16 & 0xFF;
	const auto g = color >> 8 & 0xFF;
	const auto b = color >> 0 & 0xFF;
	return (r / 2 << 16) | (g / 2 << 8) | (b / 2);
}

void kernel::terminal::type_character(char ch) {
	auto* fb = framebuffer::get_framebuffer();
	if (columns == 0)
		columns = fb->width / width;

	const auto& font_char = terminal_font[static_cast<u8>(ch)];

	for (u32 y = 0; y < height; ++y) {
		for (u32 x = 0; x < width; ++x) {
			auto& color = fb->pixels[(y + row * height) * fb->stride + (x + column * width)];
			if (get_bit(font_char[y], x)) {
				color = 0xFFFFFF;
			} else {
				color = darken_color(color);
			}
		}
	}

	column++;
	if (column >= columns) {
		column = 0;
		++row;
	}
}