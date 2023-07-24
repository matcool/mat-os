#include <kernel/window/window.hpp>
#include <kernel/window/theme.hpp>
#include <kernel/log.hpp>

using namespace kernel::window;

Window::Window(Rect rect) : window_rect(rect) {
}

void Window::add_child(WindowPtr window) {
	children.push(window);
	window->parent = this;
	window->context = context;
}

void Window::clip_bounds(bool clip_decoration) const {
	auto screen_rect = screen_window_rect();

	if (clip_decoration) {
		screen_rect = screen_client_rect();
	}

	// if theres no parent, then bounds are the same as our rect
	if (!parent) {
		context->add_clip_rect(screen_rect);
		return;
	}

	// calculate parent's bounds
	parent->clip_bounds(true);

	// intersect our bounds with parent's
	context->intersect_clip_rect(screen_rect);

	// iterate through windows above this one which are intersecting
	// TODO: a function for this or something
	// this is nasty
	usize i = 0;
	for (; i < parent->children.size(); ++i) {
        if (parent->children[i].data() == this)
            break;
	}
	for (usize j = i + 1; j < parent->children.size(); ++j) {
		auto& child = parent->children[j];
		// if it doesnt intersect the window, ignore it
		if (!window_rect.intersects(child->window_rect))
			continue;

		// if it intersects, subtract it
		context->subtract_clip_rect(child->screen_window_rect());
	}
}

void Window::paint() {
	clip_bounds();

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

	for (auto& child : children) {
		child->paint();
	}
}

void Window::draw_decoration() {
	context->draw_rect_outline(Rect(Point(0, 0), window_rect.size), theme::OUTLINE_WIDTH, theme::OUTLINE_COLOR);

	// the title bar
	context->fill(Rect(
		Point(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH),
		Point(window_rect.size.width - theme::OUTLINE_WIDTH * 2, theme::TITLEBAR_HEIGHT)
	), theme::TITLEBAR_COLOR);
	// outline below the title bar
	context->fill(Rect(
		Point(0, theme::OUTLINE_WIDTH + theme::TITLEBAR_HEIGHT),
		Point(window_rect.size.width, theme::OUTLINE_WIDTH)
	), theme::OUTLINE_COLOR);
}

Rect Window::titlebar_rect() const {
	return Rect(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH, window_rect.size.width - theme::OUTLINE_WIDTH * 2, theme::TITLEBAR_HEIGHT);
}

Rect Window::screen_window_rect() const {
	if (parent)
		return window_rect + parent->screen_client_rect().pos;
	return window_rect;
}

Rect Window::client_rect() const {
	if (!decoration)
		return Rect(Point(0, 0), window_rect.size);
	return Rect(Point(0, 0), window_rect.size - Point(theme::OUTLINE_WIDTH * 2, theme::OUTLINE_WIDTH * 3 + theme::TITLEBAR_HEIGHT));
}

static Point decoration_offset() {
	return Point(theme::OUTLINE_WIDTH, theme::OUTLINE_WIDTH * 2 + theme::TITLEBAR_HEIGHT);
}

Rect Window::relative_client_rect() const {
	if (decoration)
		return window_rect + decoration_offset();
	return window_rect;
}

Rect Window::screen_client_rect() const {
	if (decoration)
		return client_rect() + screen_window_rect().pos + decoration_offset();
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
		for (usize i = children.size(); i--; ) {
			const auto child = children[i];
			if (child->window_rect.contains(mouse_pos)) {
				if (pressed) {
					children.remove(i);
					children.push(child);

					if (child->decoration && (child->titlebar_rect() + child->window_rect.pos).contains(mouse_pos)) {
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
		drag_child->window_rect.pos = mouse_pos - drag_offset;
	}

	if (event_child) {
		const auto relative_mouse_pos = mouse_pos - event_child->relative_client_rect().pos;
		event_child->handle_mouse(relative_mouse_pos, pressed);
	} else {
		if (pressed & !last_pressed) {
			this->on_mouse_down(mouse_pos);
		}
	}

	if (!pressed)
		event_child.clear();

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
		// paint();
	}
}