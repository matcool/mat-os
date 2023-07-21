#include <kernel/screen/canvas.hpp>

Color::Color(u8 r, u8 g, u8 b) : r(r), g(g), b(b) {}

Color::Color(u32 rgb)
	: r(rgb >> 16 & 0xFF),
	g(rgb >> 8 & 0xFF),
	b(rgb & 0xFF) {}

u32 Color::packed() const {
	return (r << 16) | (g << 8) | b;
}

Canvas::Canvas(u32* pixels, usize width, usize height, usize stride)
	: m_width(width), m_height(height), m_stride(stride),
	m_pixels(pixels) {}

Canvas Canvas::sub(usize x, usize y, usize width, usize height) {
	// Canvas
	// +--------------------+
	// |                    |
	// |    Sub canvas      |
	// |    +--------+      |
	// |    |================--------...
	// ======        |      | ^ stride - width
	// | ^  +--------+      |
	// | inner_stride       |
	// |                    |
	// +--------------------+
	// inner_stride = inner_width + right_bit                   + left_bit + (stride - width)
	// inner_stride = inner_width + (width - (x + inner_width)) + (x)      + (stride - width)
	// inner_stride = inner_width + width - x - inner_width + x + stride - width
	// inner_stride = stride
	// wow!
	auto* const pixels = data() + index(x, y);
	return Canvas(pixels, width, height, stride());
}

void Canvas::paste(const Canvas& subcanvas, usize x, usize y) {
	for (usize j = 0; j < subcanvas.height() && y + j < height(); ++j) {
		for (usize i = 0; i < subcanvas.width() && x + i < width(); ++i) {
			// pixel by pixel! very slow..
			this->set(x + i, y + j, subcanvas.get(i, j));
		}
	}
}

void Canvas::set(usize x, usize y, Color color) {
	data()[index(x, y)] = color.packed();
}

Color Canvas::get(usize x, usize y) const {
	return data()[index(x, y)];
}

usize Canvas::index(usize x, usize y) const {
	return y * stride() + x;
}

void Canvas::fill(usize x, usize y, usize width, usize height, Color color) {
	for (usize j = 0; j < height && y + j < this->height(); ++j) {
		for (usize i = 0; i < width && x + i < this->width(); ++i) {
			this->set(x + i, y + j, color);
		}
	}
}