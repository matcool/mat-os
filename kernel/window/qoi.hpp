#pragma once

#include <kernel/screen/canvas.hpp>
#include <stl/array.hpp>
#include <stl/span.hpp>

// A simple QOI (Quite OK Image) format decoder, that streams every decoded pixel at a time.
class QOIStreamDecoder {
	Span<u8> m_data;

	// TODO: should be entirely 0, instead of 255 alpha
	Array<Color, 64> m_prev_pixels;
	Color m_last_pixel = Color(0, 0, 0, 255);
	u8 m_run_counter = 0;

	u32 m_width = 0;
	u32 m_height = 0;

	u8 color_hash(Color color) const;

public:
	// TODO: this should maybe take in an iterator..
	QOIStreamDecoder(Span<u8> data);

	u32 width() const { return m_width; }

	u32 height() const { return m_height; }

	// Decodes the next pixel in the image.
	Color next_pixel();

	// Returns true if there are no more pixels to be decoded.
	bool finished() const;
};