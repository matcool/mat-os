#pragma once

#include <stl/types.hpp>

namespace kernel::framebuffer {

struct Framebuffer {
	u32 width, height, stride;
	u32* pixels;
};

void init();

Framebuffer* get_framebuffer();

}