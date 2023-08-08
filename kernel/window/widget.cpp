#include <kernel/log.hpp>
#include <kernel/window/theme.hpp>
#include <kernel/window/widget.hpp>

using namespace kernel::window;

Widget::Widget(Rect rect) : m_rect(rect) {}

void Widget::add_child(WidgetPtr window) {
	m_children.push(window);
	window->m_parent = this;
	window->m_context = m_context;
	window->init();
}

usize Widget::get_child_index(Widget* child) const {
	return m_children.iter().find_value(child);
}

void Widget::reorder_child_top(Widget* child) {
	usize index = this->get_child_index(child);
	auto child_ptr = m_children[index];
	if (index == USIZE_MAX) return;
	m_children.remove(index);
	m_children.push(child_ptr);
}

void Widget::clip_bounds(bool clip_decoration, Span<const Rect> dirty_rects) {
	auto screen_rect = this->screen_rect();

	if (clip_decoration) {
		screen_rect = this->screen_client_rect();
	}

	// if theres no parent, then bounds are the same as our rect
	if (!m_parent) {
		if (dirty_rects) {
			for (const auto& rect : dirty_rects) {
				m_context->add_clip_rect(rect);
			}
			m_context->intersect_clip_rect(screen_rect);
		} else {
			m_context->add_clip_rect(screen_rect);
		}
		return;
	}

	// calculate parent's bounds
	m_parent->clip_bounds(true, dirty_rects);

	// intersect our bounds with parent's
	m_context->intersect_clip_rect(screen_rect);

	// iterate through windows above this one which are intersecting
	for (auto child : this->iter_windows_above().filter([&](auto win) {
			 return win->rect().intersects(this->rect());
		 })) {
		// if it intersects, subtract it
		m_context->subtract_clip_rect(child->screen_rect());
	}
}

void Widget::paint(Span<const Rect> dirty_rects, bool paint_children) {
	this->clip_bounds(false, dirty_rects);

	const auto screen_client = this->screen_client_rect();

	m_context->set_offset(screen_rect().pos);
	this->draw_decoration();

	m_context->intersect_clip_rect(screen_client);

	// subtract children's rects, since they will draw later
	for (auto& child : m_children) {
		m_context->subtract_clip_rect(child->screen_rect());
	}

	m_context->set_offset(screen_client.pos);
	this->draw();

	m_context->clear_clip_rects();
	m_context->set_offset(Point(0, 0));

	if (!paint_children) return;

	for (auto& child : m_children) {
		if (dirty_rects) {
			const auto screen_rect = child->screen_rect();
			// child does not intersect any of the dirty rects, so skip drawing it completely
			if (!dirty_rects.iter()
			         .map([&](const auto& rect) { return screen_rect.intersects(rect); })
			         .any()) {
				continue;
			}
		}
		child->paint(dirty_rects);
	}
}

void Widget::draw_decoration() {}

Rect Widget::rect() const {
	return m_rect;
}

Rect Widget::screen_rect() const {
	if (m_parent) return this->rect() + m_parent->screen_client_rect().pos;
	return this->rect();
}

Rect Widget::client_rect() const {
	return this->relative_client_rect().reset_pos();
}

Rect Widget::relative_client_rect() const {
	return this->rect().reset_pos();
}

Rect Widget::screen_client_rect() const {
	return this->relative_client_rect() + this->screen_rect().pos;
}

void Widget::draw() {
	m_context->fill(this->client_rect(), Color(255, 0, 0));
}

void Widget::handle_mouse(Point mouse_pos, bool pressed) {
	// mouse position relative to the client rect
	const auto client_mouse_pos = mouse_pos - this->relative_client_rect().pos;

	if (pressed && !m_last_pressed) {
		m_focus_child.clear();

		// if the client rect does not contain the mouse, then the click is in the widget
		// decoration, so dont send the event to a child
		if (this->relative_client_rect().contains(mouse_pos)) {
			// TODO: reverse iterator
			for (usize i = m_children.size(); i--;) {
				auto child = m_children[i];
				if (child->rect().contains(client_mouse_pos)) {
					m_event_child = child;
					m_focus_child = child;
					break;
				}
			}
		}

		// Send on focus event regardless if child will eat mouse events
		this->on_focus();
	}

	// Prioritize sending the event to a child
	if (m_event_child) {
		const auto relative_mouse_pos = client_mouse_pos - m_event_child->rect().pos;
		m_event_child->handle_mouse(relative_mouse_pos, pressed);
	} else if (pressed & !m_last_pressed) {
		this->on_mouse_down(mouse_pos);
	} else if (!pressed && m_last_pressed) {
		this->on_mouse_up(mouse_pos);
	} else {
		this->on_mouse_move(mouse_pos);
	}

	if (!pressed) m_event_child.clear();

	m_last_pressed = pressed;
}

void Widget::handle_keyboard(ps2::Key key, bool pressed) {
	if (m_focus_child) {
		m_focus_child->handle_keyboard(key, pressed);
	} else if (pressed) {
		this->on_key_press(key);
	}
}

void Widget::move_to(const Point& pos) {
	if (!m_parent) return;

	this->clip_bounds(false);

	const auto new_window_rect =
		m_rect.with_pos(pos) + (m_parent ? m_parent->screen_client_rect().pos : Point(0, 0));

	m_context->subtract_clip_rect(new_window_rect);

	// this is quite hacky too
	const auto dirty_rects = m_context->get_clip_rects();
	m_context->clear_clip_rects();

	const auto old_rect = m_rect;
	m_rect.pos = pos;

	// Paint every window below this one which intersected it
	for (auto& sibling : this->iter_windows_below().filter([&](const auto& win) {
			 return win->rect().intersects(old_rect);
		 })) {
		sibling->paint(dirty_rects.span(), true);
	}

	m_parent->paint(dirty_rects.span(), false);

	this->paint();
}

void Widget::resize(const Point& new_size) {
	m_rect.size = new_size;
}

void Widget::invalidate(const Rect& rect) {
	const auto offset_rect = rect + this->screen_rect().pos;
	const auto rects = Span(&offset_rect, 1);
	// dont paint children, since this should only invalidate the window itself
	this->paint(rects, false);
}