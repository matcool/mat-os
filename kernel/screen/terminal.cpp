#include <stl/math.hpp>
#include <kernel/screen/terminal.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/screen/terminal_font.hpp>
#include <kernel/log.hpp>

using math::get_bit;

static constexpr u32 scale = 2;
static constexpr u32 width = 7 * scale;
static constexpr u32 height = 10 * scale;
u32 columns = 0;
u32 column = 0;
u32 row = 0;

void kernel::terminal::type_character(char ch) {
	auto* fb = &framebuffer::get_framebuffer();
	if (!fb->data()) return;

	if (columns == 0)
		columns = fb->width() / width;
	if (row >= fb->height() / height)
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
		fb->fill(column * width, row * height, width, height, Color(0));
		return;
	}

	const auto& font_char = terminal_font[static_cast<u8>(ch)];

	for (u32 y = 0; y < height; ++y) {
		for (u32 x = 0; x < width; ++x) {
			const auto pix_x = x + column * width;
			const auto pix_y = y + row * height;
			if (get_bit(font_char[y / scale], x / scale)) {
				fb->set(pix_x, pix_y, Color(255, 255, 255));
			} else {
				fb->set(pix_x, pix_y, Color(0, 0, 0));
			}
		}
	}

	column++;
	if (column >= columns) {
		column = 0;
		++row;
	}
}

void kernel::terminal::go_to(u32 row_, u32 column_) {
	row = row_;
    column = column_;
}