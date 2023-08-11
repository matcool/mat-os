#include <kernel/log.hpp>
#include <kernel/window/font.hpp>
#include <kernel/window/qoi.hpp>
#include <stl/math.hpp>

namespace kernel::window {

BitmapFont BitmapFont::from_qoi(Span<const u8> data) {
	QOIStreamDecoder decoder(data);

	// much easier to just dump the image somewhere first, instead of streaming it
	Vector<Color> image;

	image.reserve(decoder.width() * decoder.height());

	while (!decoder.finished()) {
		image.push(decoder.next_pixel());
	}

	BitmapFont font(decoder.width() / 16, decoder.height() / 16);

	// u8 counter = 0;
	// u8 value = 0;
	for (const auto ch : iterators::range(256)) {
		const auto row = ch / 16;
		const auto col = ch % 16;
		for (const auto y : iterators::range(font.char_height())) {
			for (const auto x : iterators::range(font.char_width())) {
				const auto pix_x = col * font.char_width() + x;
				const auto pix_y = row * font.char_height() + y;
				const auto& pixel = image[pix_y * decoder.width() + pix_x];
				const bool is_set = pixel.r > 127;
				// value = (value << 1) | is_set;
				// if (++counter == 8) {
				// 	font.m_data.push(value);
				// 	counter = value = 0;
				// }
				font.m_data.push(is_set);
			}
		}
	}
	// if (counter) font.m_data.push(value);

	return font;
}

bool BitmapFont::is_char_pixel_set(u8 ch, u16 x, u16 y) const {
	const auto char_size = this->char_width() * this->char_height();
	const auto bit_index = char_size * ch + y * this->char_width() + x;
	// const auto byte_index = bit_index / 8;
	// const auto byte_offset = bit_index % 8;
	// return math::get_bit(m_data[byte_index], byte_offset);
	return m_data[bit_index];
}

}