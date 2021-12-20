#pragma once
#include "common.hpp"

class Screen {
public:
	static auto& get() {
		static Screen instance;
		return instance;
	}

	u32 width, height;
	u32* raw_buffer;

	void redraw();

	// color is in the format 0xAARRGGB, although
	// the alpha makes no sense in the context of a screen
	inline void set_pixel(u32 x, u32 y, u32 color) {
		raw_buffer[y * width + x] = color;
	}
};
