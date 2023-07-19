#include <stl/random.hpp>
#include <kernel/screen/terminal.hpp>
#include <kernel/screen/framebuffer.hpp>

const u32 width = 10;
const u32 height = 20;
u32 columns = 0;
u32 column = 0;
u32 row = 0;

void kernel::terminal::type_character(char ch) {
	auto* fb = framebuffer::get_framebuffer();
	if (columns == 0)
		columns = fb->width / width;
	mat::random::Generator rng(ch);
	const auto color = rng.range(0, 0xffffff + 1);

	for (u32 y = 0; y < height; ++y) {
		for (u32 x = 0; x < width; ++x) {
			fb->pixels[(y + row * height) * fb->stride + (x + column * width)] = color;
		}
	}

	column++;
	if (column >= columns) {
		column = 0;
		++row;
	}
}