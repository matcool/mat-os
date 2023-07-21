#pragma once

#include <stl/types.hpp>
#include <kernel/screen/canvas.hpp>

namespace kernel::framebuffer {

void init();

Canvas* get_framebuffer();

}