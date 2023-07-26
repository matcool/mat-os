#include <kernel/log.hpp>
#include <kernel/window/theme.hpp>
#include <kernel/window/window.hpp>

using namespace kernel::window;

Window::Window(Rect rect) : window_rect(rect) {}

void Window::add_child(WindowPtr window) {
	children.push(window);
	window->parent = this;
	window->context = context;
}

void Window::clip_bounds(bool clip_decoration, Span<const Rect> dirty_rects) {
	auto screen_rect = screen_window_rect();

	if (clip_decoration) {
		screen_rect = screen_client_rect();
	}

	// if theres no parent, then bounds are the same as our rect
	if (!parent) {
		if (dirty_rects) {
			for (const auto& rect : dirty_rects) {
				context->add_clip_rect(rect);
			}
			context->intersect_clip_rect(screen_rect);
		} else {
			context->add_clip_rect(screen_rect);
		}
		return;
	}

	// calculate parent's bounds
	parent->clip_bounds(true, dirty_rects);

	// intersect our bounds with parent's
	context->intersect_clip_rect(screen_rect);

	// iterate through windows above this one which are intersecting
	for (auto child : iter_windows_above().filter([&](auto win) {
			 return win->window_rect.intersects(window_rect);
		 })) {
		// if it intersects, subtract it
		context->subtract_clip_rect(child->screen_window_rect());
	}
}

void Window::paint(Span<const Rect> dirty_rects, bool paint_children) {
	clip_bounds(false, dirty_rects);

	const auto screen_pos = screen_client_rect().pos;

	if (decoration) {
		context->set_offset(screen_window_rect().pos);
		draw_decoration();

		context->intersect_clip_rect(screen_client_rect());
	}

	// subtract children's rects, since they will draw later
	for (auto& child : children) {
		context->subtract_clip_rect(child->screen_window_rect());
	}

	context->set_offset(screen_pos);
	draw();

	context->clear_clip_rects();
	context->set_offset(Point(0, 0));

	if (!paint_children) return;

	for (auto& child : children) {
		if (dirty_rects) {
			const auto screen_rect = child->screen_window_rect();
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

void Window::draw_decoration() {
	context->draw_rect_outline(
		window_rect.with_pos(Point(0, 0)), theme::OUTLINE_WIDTH, theme::OUTLINE_COLOR
	);

	const auto outline_offset = Point(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH);

	// the title bar
	context->fill(
		Rect(
			outline_offset,
			Point(window_rect.size.width - theme::OUTLINE_WIDTH * 2, theme::TITLEBAR_HEIGHT)
		),
		theme::TITLEBAR_COLOR
	);

	context->draw_text(title, outline_offset + Point(5, 5), theme::TITLE_TEXT_COLOR);

	// outline below the title bar
	context->fill(
		Rect(
			Point(0, theme::OUTLINE_WIDTH + theme::TITLEBAR_HEIGHT),
			Point(window_rect.size.width, theme::OUTLINE_WIDTH)
		),
		theme::OUTLINE_COLOR
	);
}

Rect Window::titlebar_rect() const {
	return Rect(
		theme::OUTLINE_WIDTH,
		theme::OUTLINE_WIDTH,
		window_rect.size.width - theme::OUTLINE_WIDTH * 2,
		theme::TITLEBAR_HEIGHT
	);
}

Rect Window::screen_window_rect() const {
	if (parent) return window_rect + parent->screen_client_rect().pos;
	return window_rect;
}

Rect Window::client_rect() const {
	if (!decoration) return window_rect.with_pos(Point(0, 0));
	return Rect(
		Point(0, 0),
		window_rect.size -
			Point(theme::OUTLINE_WIDTH * 2, theme::OUTLINE_WIDTH * 3 + theme::TITLEBAR_HEIGHT)
	);
}

static Point decoration_offset() {
	return Point(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH * 2 + theme::TITLEBAR_HEIGHT);
}

Rect Window::relative_client_rect() const {
	if (decoration) return window_rect + decoration_offset();
	return window_rect;
}

Rect Window::screen_client_rect() const {
	if (decoration) return client_rect() + screen_window_rect().pos + decoration_offset();
	return screen_window_rect();
}

void Window::draw() {
	context->fill(client_rect(), theme::WINDOW_COLOR);
}

void Window::on_mouse_down(Point) {
	return;
}

void Window::handle_mouse(Point mouse_pos, bool pressed) {
	if (pressed && !last_pressed) {
		for (usize i = children.size(); i--;) {
			auto child = children[i];
			if (child->window_rect.contains(mouse_pos)) {
				if (pressed) {
					child->raise();

					if (child->decoration &&
					    (child->titlebar_rect() + child->window_rect.pos).contains(mouse_pos)) {
						drag_child = child;
						drag_offset = mouse_pos - child->window_rect.pos;
					}
				}
				if (!drag_child) {
					event_child = child;
				}
				break;
			}
		}
	} else if (!pressed) {
		drag_child.clear();
	}

	if (drag_child) {
		drag_child->move_to(mouse_pos - drag_offset);
	}

	if (event_child) {
		const auto relative_mouse_pos = mouse_pos - event_child->relative_client_rect().pos;
		event_child->handle_mouse(relative_mouse_pos, pressed);
	} else {
		if (pressed & !last_pressed) {
			this->on_mouse_down(mouse_pos);
		}
	}

	if (!pressed) event_child.clear();

	last_pressed = pressed;
}

void Window::raise(bool redraw) {
	if (!parent) return;
	if (parent->focus_child.data() == this) return;
	auto previous = parent->focus_child;

	usize i = 0;
	for (; i < parent->children.size(); ++i) {
		if (parent->children[i].data() == this) {
			break;
		}
	}
	auto self = parent->children[i];
	parent->children.remove(i);
	parent->children.push(self);

	parent->focus_child = self;

	if (redraw) {
		paint();
	}
}

void Window::move_to(const Point& pos) {
	raise(false);

	clip_bounds(false);

	const auto new_window_rect =
		window_rect.with_pos(pos) + (parent ? parent->screen_client_rect().pos : Point(0, 0));

	context->subtract_clip_rect(new_window_rect);

	// this is quite hacky too
	const auto dirty_rects = context->get_clip_rects();
	context->clear_clip_rects();

	const auto old_rect = window_rect;
	window_rect.pos = pos;

	// Paint every window below this one which intersected it
	for (auto& sibling : iter_windows_below().filter([&](const auto& win) {
			 return win->window_rect.intersects(old_rect);
		 })) {
		sibling->paint(dirty_rects.span(), true);
	}

	parent->paint(dirty_rects.span(), false);

	paint();
}

void Window::invalidate(const Rect& rect) {
	const auto offset_rect = rect + screen_window_rect().pos;
	const auto rects = Span(&offset_rect, 1);
	// dont paint children, since this should only invalidate the window itself
	paint(rects, false);
}

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