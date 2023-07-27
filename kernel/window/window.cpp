#include <kernel/log.hpp>
#include <kernel/window/theme.hpp>
#include <kernel/window/widget.hpp>

using namespace kernel::window;

void Window::draw() {
	context->fill(client_rect(), theme::WINDOW_COLOR);
}

void Window::draw_decoration() {
	context->draw_rect_outline(
		rect().with_pos(Point(0, 0)), theme::OUTLINE_WIDTH, theme::OUTLINE_COLOR
	);

	const auto outline_offset = Point(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH);

	// the title bar
	context->fill(
		Rect(outline_offset, Point(rect().size.width - theme::OUTLINE_WIDTH * 2, theme::TITLEBAR_HEIGHT)),
		theme::TITLEBAR_COLOR
	);

	context->draw_text(title, outline_offset + Point(5, 5), theme::TITLE_TEXT_COLOR);

	// outline below the title bar
	context->fill(
		Rect(
			Point(0, theme::OUTLINE_WIDTH + theme::TITLEBAR_HEIGHT),
			Point(rect().size.width, theme::OUTLINE_WIDTH)
		),
		theme::OUTLINE_COLOR
	);
}

Rect Window::relative_client_rect() const {
	const auto offset =
		Point(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH * 2 + theme::TITLEBAR_HEIGHT);
	return Rect::from_corners(
		offset, rect().reset_pos().bot_right() - Point(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH)
	);
}

void Window::on_mouse_down(Point p) {
	kdbgln("Window::on_mouse_down {},{} - {}", p.x, p.y, title);
	if (p.y < 30) {
		dragging = true;
		drag_offset = p;
	}
	return;
}

void Window::on_mouse_up(Point p) {
	kdbgln("Window::on_mouse_up {},{} - {}", p.x, p.y, title);
	dragging = false;
	return;
}

void Window::on_mouse_move(Point p) {
	if (dragging) this->move_to(rect().pos + p - drag_offset);
	return;
}

void Window::on_focus() {
	this->raise();
}

void Window::raise(bool redraw) {
	if (!parent) return;

	parent->reorder_child_top(this);

	if (redraw) this->paint();
}