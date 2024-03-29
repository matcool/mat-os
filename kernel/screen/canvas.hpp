#pragma once

#include <stl/math.hpp>
#include <stl/types.hpp>

// Represents a RGBA8888 color
struct Color {
	union {
		struct {
			u8 b, g, r, a;
		};

		u32 packed;
	};

	constexpr Color(u8 r, u8 g, u8 b, u8 a = 255) : b(b), g(g), r(r), a(a) {}

	constexpr Color(u32 argb) : packed(argb) {}

	constexpr Color() : Color(0, 0, 0) {}

	constexpr static Color from_hex(u32 rgb) { return Color(rgb | 0xFF000000); }
};

// Represents a pixel buffer, with a given width and height.
// Assumes the pixels are in a linear buffer, may be changed later.
class Canvas {
	usize m_width = 0;
	usize m_height = 0;
	// The stride indicates how many pixels to skip per row.
	// Typically is the same as the width, but can be different for easy sub canvases.
	usize m_stride = 0;
	// TODO: better type?
	u32* m_pixels = nullptr;

public:
	Canvas(u32* pixels, usize width, usize height) : Canvas(pixels, width, height, width) {}

	Canvas(u32* pixels, usize width, usize height, usize stride);

	auto width() const { return m_width; }

	auto height() const { return m_height; }

	auto stride() const { return m_stride; }

	// Returns the raw pixel data
	auto* data() { return m_pixels; }

	const auto* data() const { return m_pixels; }

	// Returns a subcanvas (*with the same pixels!*) at offset (x, y) and size (width, height).
	// This is a cheap operation, since it will point to the same data.
	Canvas sub(usize x, usize y, usize width, usize height);

	// Pastes a smaller subcanvas at offset (x, y), which will be the top-left of the sub-canvas.
	// TODO: what should happen if it overlaps?
	// TODO: what should happen if subcanvas is bigger?
	void paste(const Canvas& subcanvas, usize x, usize y);

	// Same as paste(), but will not replace pixels if their alpha is zero.
	// This will not do alpha blending, only 1-bit alpha masking.
	void paste_alpha_masked(const Canvas& subcanvas, usize x, usize y);

	// Sets a pixel at (x, y) to a color.
	void set(usize x, usize y, Color color);

	// Gets a color at (x, y).
	Color get(usize x, usize y) const;

	// Gets the pixel index for (x, y).
	usize index(usize x, usize y) const;

	// Fills the rect (x, y, width, height) with color.
	void fill(usize x, usize y, usize width, usize height, Color color);

	// Fills a given rect. If the position is negative, clip the rect properly.
	template <class Type>
	void fill(math::Rect<Type> rect, Color color) {
		if (rect.pos.x < 0) {
			rect.size.width += rect.pos.x;
			rect.pos.x = 0;
		}
		if (rect.pos.y < 0) {
			rect.size.height += rect.pos.y;
			rect.pos.y = 0;
		}
		if (rect.size.width < 0 || rect.size.height < 0) return;
		this->fill(rect.pos.x, rect.pos.y, rect.size.width, rect.size.height, color);
	}
};