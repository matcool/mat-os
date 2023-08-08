#include <kernel/window/qoi.hpp>

u8 QOIStreamDecoder::color_hash(Color color) const {
	return (color.r * 3 + color.g * 5 + color.b * 7 + color.a * 11) % 64;
}

QOIStreamDecoder::QOIStreamDecoder(Span<u8> data) : m_data(data) {
	// skip magic bytes
	m_data = m_data.sub(4);

	auto width_bytes = m_data.sub(0, 4);
	m_data = m_data.sub(4);
	m_width = static_cast<u32>(width_bytes[0]) << 24 | static_cast<u32>(width_bytes[1]) << 16 |
		static_cast<u32>(width_bytes[2]) << 8 | static_cast<u32>(width_bytes[3]);

	auto height_bytes = m_data.sub(0, 4);
	m_data = m_data.sub(4);
	m_height = static_cast<u32>(height_bytes[0]) << 24 | static_cast<u32>(height_bytes[1]) << 16 |
		static_cast<u32>(height_bytes[2]) << 8 | static_cast<u32>(height_bytes[3]);

	// ignore colorspace and channels
	m_data = m_data.sub(8);
}

static constexpr u8 QOI_OP_INDEX = 0b0000'0000;
static constexpr u8 QOI_OP_DIFF = 0b0100'0000;
static constexpr u8 QOI_OP_LUMA = 0b1000'0000;
static constexpr u8 QOI_OP_RUN = 0b1100'0000;
static constexpr u8 QOI_OP_RGB = 0b1111'1110;
static constexpr u8 QOI_OP_RGBA = 0b1111'1111;

static constexpr u8 QOI_OP_MASK = 0b1100'0000;

Color QOIStreamDecoder::next_pixel() {
	if (m_run_counter) {
		--m_run_counter;
		return m_last_pixel;
	}

	Color color = m_last_pixel;

	const auto byte = m_data[0];
	m_data = m_data.sub(1);

	const auto masked = byte & QOI_OP_MASK;
	const auto no_tag = byte & 0b0011'1111;

	if (byte == QOI_OP_RGB || byte == QOI_OP_RGBA) {
		color.r = m_data[0];
		m_data = m_data.sub(1);
		color.g = m_data[0];
		m_data = m_data.sub(1);
		color.b = m_data[0];
		m_data = m_data.sub(1);
		if (byte == QOI_OP_RGBA) {
			color.a = m_data[0];
			m_data = m_data.sub(1);
		}
	} else if (masked == QOI_OP_INDEX) {
		color = m_prev_pixels[byte];
	} else if (masked == QOI_OP_DIFF) {
		i8 diff_r = (no_tag >> 4) - 2;
		i8 diff_g = (no_tag >> 2) - 2;
		i8 diff_b = (no_tag >> 0) - 2;
		color.r += diff_r;
		color.g += diff_g;
		color.b += diff_b;
	} else if (masked == QOI_OP_LUMA) {
		i8 diff_g = no_tag - 32;
		const auto byte = m_data[0];
		m_data = m_data.sub(1);
		i8 diff_r = diff_g + (byte >> 4) - 8;
		i8 diff_b = diff_g + (byte >> 0) - 8;
		color.r += diff_r;
		color.g += diff_g;
		color.b += diff_b;
	} else if (masked == QOI_OP_RUN) {
		m_run_counter = no_tag + 1;
	}

	m_last_pixel = color;
	m_prev_pixels[this->color_hash(color)] = color;

	return color;
}

bool QOIStreamDecoder::finished() const {
	if (!m_data || m_data.size() < 8) return true;
	// if the next 7 bytes are 0 and the one after is a 1, then qoi's stream is finished.
	return !m_data.iter().take(7).any() && m_data[7] == 1;
}