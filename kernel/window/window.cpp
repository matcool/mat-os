#include <kernel/log.hpp>
#include <kernel/window/theme.hpp>
#include <kernel/window/widget.hpp>

using namespace kernel::window;

Window::Window(Rect rect, StringView title) :
	Widget(Rect(
		rect.pos,
		rect.size + Point(theme::OUTLINE_WIDTH * 2, theme::OUTLINE_WIDTH * 3 + theme::TITLEBAR_HEIGHT)
	)),
	m_title(title) {}

void Window::draw() {
	m_context->fill(client_rect(), theme::WINDOW_COLOR);
}

void Window::draw_decoration() {
	m_context->draw_rect_outline(
		rect().with_pos(Point(0, 0)), theme::OUTLINE_WIDTH, theme::OUTLINE_COLOR
	);

	const auto outline_offset = Point(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH);

	// the title bar
	m_context->fill(
		Rect(outline_offset, Point(rect().size.width - theme::OUTLINE_WIDTH * 2, theme::TITLEBAR_HEIGHT)),
		theme::TITLEBAR_COLOR
	);

	m_context->draw_text(m_title, outline_offset + Point(5, 5), theme::TITLE_TEXT_COLOR);

	// outline below the title bar
	m_context->fill(
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
	if (p.y < 30) {
		m_dragging = true;
		m_drag_offset = p;
	}
	return;
}

void Window::on_mouse_up(Point) {
	m_dragging = false;
	return;
}

void Window::on_mouse_move(Point p) {
	if (m_dragging) this->move_to(rect().pos + p - m_drag_offset);
	return;
}

void Window::on_focus() {
	this->raise();
}

void Window::raise(bool redraw) {
	if (!m_parent) return;

	m_parent->reorder_child_top(this);

	if (redraw) this->paint();
}