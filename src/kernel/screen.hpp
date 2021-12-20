#pragma once
#include "common.hpp"

class Screen {
public:
	static auto& get() {
		static Screen instance;
		return instance;
	}

	// TODO: move this to the ctor somehow
	void init(u32 width, u32 height, u32*);

	u32 width, height;
	u32* buffer_a;
	u32* buffer_b;

	void redraw();

	// color is in the format 0xAARRGGB, although
	// the alpha makes no sense in the context of a screen
	inline void set_pixel(u32 x, u32 y, u32 color) {
		buffer_b[y * width + x] = color;
	}
};
