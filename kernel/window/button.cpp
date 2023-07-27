#include <kernel/log.hpp>
#include <kernel/window/widget.hpp>

using namespace kernel::window;

void Button::draw() {
	const auto rect = client_rect();
	context->fill(rect, Color(0));
	context->fill(
		Rect::from_corners(Point(1, 1), rect.bot_right() - Point(1, 1)), Color(255, 255, 255)
	);
	context->fill(
		Rect::from_corners(rect.mid_point() - Point(5, 5), rect.mid_point() + Point(5, 5)),
		active ? Color(50, 200, 50) : Color(255, 100, 100)
	);
}

void Button::on_mouse_down(Point) {
	active = !active;
	invalidate(client_rect());
}