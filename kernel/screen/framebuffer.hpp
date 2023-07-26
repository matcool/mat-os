#pragma once

#include <kernel/screen/canvas.hpp>
#include <stl/types.hpp>

namespace kernel::framebuffer {

void init();

Canvas& get_framebuffer();

void loop();

}