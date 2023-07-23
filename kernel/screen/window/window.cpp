#include <kernel/screen/window/manager.hpp>

using namespace kernel::window;

static constexpr Color window_color = Color(200, 200, 200);
static constexpr Color outline_color = Color(0, 0, 0);

Window::Window(Rect rect) : rect(rect) {
}

void Window::paint(WindowContext& context) {
	static constexpr i32 outline_width = 1;
	context.fill(Rect::from_corners(rect.top_left(), rect.top_right() + Point(0, outline_width)), outline_color);
	context.fill(Rect::from_corners(rect.top_left(), rect.bot_left() + Point(outline_width, 0)), outline_color);
	context.fill(Rect::from_corners(rect.top_right() - Point(outline_width, 0), rect.bot_right()), outline_color);
	context.fill(Rect::from_corners(rect.bot_left() - Point(0, outline_width), rect.bot_right()), outline_color);

	context.fill(Rect::from_corners(rect.top_left() + outline_width, rect.bot_right() - outline_width), window_color);
}