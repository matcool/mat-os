#include <kernel/screen/window/window.hpp>

using namespace kernel::window;

Window::Window(Rect rect) : window_rect(rect) {
}

void Window::add_child(WindowPtr window) {
	children.push(window);
	window->parent = this;
}

void Window::clip_bounds(WindowContext& context, bool clip_decoration) const {
	auto screen_rect = screen_window_rect();

	if (clip_decoration) {
		screen_rect = screen_client_rect();
	}

	// if theres no parent, then bounds are the same as our rect
	if (!parent) {
		context.add_clip_rect(screen_rect);
		return;
	}

	// calculate parent's bounds
	parent->clip_bounds(context, true);

	// intersect our bounds with parent's
	context.intersect_clip_rect(screen_rect);

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
		context.subtract_clip_rect(child->screen_window_rect());
	}
}

void Window::paint(WindowContext& context) {
	clip_bounds(context);

	const auto screen_pos = screen_client_rect().pos;

	if (decoration) {
		context.set_offset(screen_window_rect().pos);
		draw_decoration(context);
		
		context.intersect_clip_rect(screen_client_rect());
	}

    // subtract children's rects, since they will draw later
	for (auto& child : children) {
		context.subtract_clip_rect(child->screen_window_rect());
	}

	context.set_offset(screen_pos);
	draw(context);

	context.clear_clip_rects();
	context.set_offset(Point(0, 0));

	for (auto& child : children) {
		child->paint(context);
	}
}

static constexpr i32 outline_width = 3;
static constexpr i32 titlebar_height = 20;
static constexpr Color window_color = Color(200, 200, 200);
static constexpr Color outline_color = Color(0, 0, 0);
static constexpr Color titlebar_color = Color(200, 100, 100);

void Window::draw_decoration(WindowContext &context) {
	context.fill(Rect(Point(0, 0), Point(window_rect.size.width, outline_width)), outline_color);
	context.fill(Rect(Point(0, 0), Point(outline_width, window_rect.size.height)), outline_color);
	context.fill(Rect(Point(0, window_rect.size.height - outline_width), Point(window_rect.size.width, outline_width)), outline_color);
	context.fill(Rect(Point(window_rect.size.width - outline_width, 0), Point(outline_width, window_rect.size.height)), outline_color);

	context.fill(Rect(Point(outline_width, outline_width), Point(window_rect.size.width - outline_width * 2, titlebar_height)), titlebar_color);
	context.fill(Rect(Point(0, outline_width + titlebar_height), Point(window_rect.size.width, outline_width)), outline_color);
}

Rect Window::titlebar_rect() const {
	return Rect(outline_width, outline_width, window_rect.size.width - outline_width, titlebar_height);
}

Rect Window::screen_window_rect() const {
	if (parent)
		return window_rect + parent->screen_client_rect().pos;
	return window_rect;
}

Rect Window::client_rect() const {
	if (!decoration)
		return Rect(Point(0, 0), window_rect.size);
	return Rect(Point(0, 0), window_rect.size - Point(outline_width * 2, outline_width * 3 + titlebar_height));
}

static Point decoration_offset() {
	return Point(outline_width, outline_width * 2 + titlebar_height);
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

void Window::draw(WindowContext& context) {
	context.fill(client_rect(), window_color);
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
					active_child = child;
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

	if (active_child) {
		const auto relative_mouse_pos = mouse_pos - active_child->relative_client_rect().pos;
		active_child->handle_mouse(relative_mouse_pos, pressed);
	} else {
		if (pressed & !last_pressed) {
			this->on_mouse_down(mouse_pos);
		}
	}

	if (!pressed)
		active_child.clear();

	last_pressed = pressed;
}