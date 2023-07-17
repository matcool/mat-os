#pragma once

#include <stl/types.hpp>

namespace kernel::ps2 {

static constexpr u16 PS2_DATA_PORT = 0x60;
static constexpr u16 PS2_COM_PORT = 0x64;

void init();

void handle_keyboard();

}