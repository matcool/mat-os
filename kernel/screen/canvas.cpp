#include <stl/memory.hpp>
#include <kernel/screen/canvas.hpp>

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
		// assume rows are linear on both canvases..
		auto* src = &subcanvas.data()[subcanvas.index(0, j)];
		auto* dst = &data()[index(x, y + j)];
		memcpy(dst, src, subcanvas.width() * sizeof(Color));
	}
}

void Canvas::paste_alpha_masked(const Canvas& subcanvas, usize x, usize y) {
	for (usize j = 0; j < subcanvas.height() && y + j < height(); ++j) {
		for (usize i = 0; i < subcanvas.width() && x + i < width(); ++i) {
			const auto color = subcanvas.get(i, j);
			if (color.a)
				this->set(x + i, y + j, color);
		}
	}
}

void Canvas::set(usize x, usize y, Color color) {
	data()[index(x, y)] = color.packed;
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