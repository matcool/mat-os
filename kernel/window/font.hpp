#pragma once

#include <stl/types.hpp>
#include <stl/vector.hpp>

namespace kernel::window {

// Represents a simple monospace bitmap font which can only be transparent or fully white.
class BitmapFont {
	Vector<u8> m_data;
	u16 m_width = 0;
	u16 m_height = 0;

	BitmapFont(u16 width, u16 height) : m_width(width), m_height(height) {}

public:
	// Creates a new font from raw QOI data.
	static BitmapFont from_qoi(Span<const u8> data);

	u16 char_width() const { return m_width; }

	u16 char_height() const { return m_height; }

	// Checks if for some character `ch` its pixel at (x, y) is set.
	bool is_char_pixel_set(u8 ch, u16 x, u16 y) const;

	bool is_char_pixel_set(char ch, u16 x, u16 y) const {
		return is_char_pixel_set(static_cast<u8>(ch), x, y);
	}
};

}