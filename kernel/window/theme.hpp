#pragma once

#include <kernel/screen/canvas.hpp>

namespace kernel::window::theme {

// Window styles
static constexpr i32 OUTLINE_WIDTH = 2;
static constexpr i32 TITLEBAR_HEIGHT = 20;
static constexpr Color WINDOW_COLOR = Color::from_hex(0xc8c8c8);
static constexpr Color OUTLINE_COLOR = Color::from_hex(0x000000);
static constexpr Color TITLEBAR_COLOR = Color::from_hex(0xc86464);
static constexpr Color TITLE_TEXT_COLOR = Color::from_hex(0x000000);

// Desktop styles
static constexpr Color DESKTOP_COLOR = Color::from_hex(0x6464c8);

}