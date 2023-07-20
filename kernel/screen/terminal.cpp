#include <stl/math.hpp>
#include <kernel/screen/terminal.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/screen/terminal_font.hpp>
#include <kernel/log.hpp>

using mat::math::get_bit;

static constexpr u32 scale = 2;
static constexpr u32 width = 7 * scale;
static constexpr u32 height = 10 * scale;
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
	if (!fb->pixels) return;
	if (columns == 0)
		columns = fb->width / width;
	if (row >= fb->height / height)
		row = 0;
	
	if (ch == '\n') {
		column = 0;
		row++;
		return;
	} else if (ch == '\x08') {
		if (column != 0) {
			column--;
		} else if (row != 0) {
			row--;
			column = columns - 1;
		}
		for (u32 y = 0; y < height; ++y) {
			for (u32 x = 0; x < width; ++x) {
				fb->pixels[(y + row * height) * fb->stride + (x + column * width)] = 0;
			}
		}
		return;
	}

	const auto& font_char = terminal_font[static_cast<u8>(ch)];

	for (u32 y = 0; y < height; ++y) {
		for (u32 x = 0; x < width; ++x) {
			auto& color = fb->pixels[(y + row * height) * fb->stride + (x + column * width)];
			if (get_bit(font_char[y / scale], x / scale)) {
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