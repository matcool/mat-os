#include <kernel/screen/window/manager.hpp>

using namespace kernel::window;

Window::Window(Rect rect) : rect(rect) {
}

void Window::handle_mouse(Point mouse_pos, bool pressed) {
	if (pressed && !last_pressed) {
		for (usize i = children.size(); i--; ) {
			const auto child = children[i];
			const auto screen_rect = child->screen_rect();
			if (screen_rect.contains(mouse_pos) && child->draggable) {
				children.remove(i);
				children.push(child);

				drag_child = child;
				drag_offset = mouse_pos - screen_rect.pos;
				break;
			}
		}
	} else if (!pressed) {
		drag_child.clear();
	}

	if (drag_child) {
		drag_child->rect.pos = mouse_pos - drag_offset - parent_screen_pos();
	}

	last_pressed = pressed;
}

void Window::add_child(WindowPtr window) {
	children.push(window);
	window->parent = this;
}

Point Window::parent_screen_pos() const {
	if (parent)
		return parent->screen_pos();
	return Point(0, 0);
}

Point Window::screen_pos() const {
	return rect.pos + parent_screen_pos();
}

void Window::clip_bounds(WindowContext& context) const {
	const auto screen_rect = this->screen_rect();

	// if theres no parent, then bounds are the same as our rect
	if (!parent) {
		context.add_clip_rect(screen_rect);
		return;
	}

	// calculate parent's bounds
	parent->clip_bounds(context);

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
		if (!rect.intersects(child->rect))
			continue;

		// if it intersects, subtract it
		context.subtract_clip_rect(child->screen_rect());
	}
}

void Window::paint(WindowContext& context) {
	clip_bounds(context);

	auto screen_pos = this->screen_pos();

	if (decoration) {
		// at this point this should only ever be (0, 0)
		// but just in case
		const auto prev_offset = context.offset;
		context.offset = screen_pos;
		draw_decoration(context);
		context.offset = prev_offset;

		screen_pos += decoration_offset();
		
		context.intersect_clip_rect(view_rect() + screen_pos);
	}

    // subtract children's rects, since they will draw later
	for (auto& child : children) {
		context.subtract_clip_rect(child->screen_rect());
	}

	context.offset = screen_pos;
	draw(context);

	context.clear_clip_rects();
	context.offset = Point(0, 0);

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
	context.fill(Rect(Point(0, 0), Point(rect.size.width, outline_width)), outline_color);
	context.fill(Rect(Point(0, 0), Point(outline_width, rect.size.height)), outline_color);
	context.fill(Rect(Point(0, rect.size.height - outline_width), Point(rect.size.width, outline_width)), outline_color);
	context.fill(Rect(Point(rect.size.width - outline_width, 0), Point(outline_width, rect.size.height)), outline_color);

	context.fill(Rect(Point(outline_width, outline_width), Point(rect.size.width - outline_width * 2, titlebar_height)), titlebar_color);
	context.fill(Rect(Point(0, outline_width + titlebar_height), Point(rect.size.width, outline_width)), outline_color);
}

Point Window::decoration_offset() const {
	return Point(outline_width, outline_width * 2 + titlebar_height);
}

Rect Window::view_rect() const {
	if (!decoration)
		return Rect(Point(0, 0), rect.size);
	
	return Rect(Point(0, 0), rect.size - Point(outline_width * 2, outline_width * 3 + titlebar_height));
}

void Window::draw(WindowContext& context) {
	const auto rect = view_rect();
	context.fill(rect, window_color);
	if (!decoration) {
		// draw 1 pixel outline to make sure our bounds are correct
		context.fill(Rect::from_corners(rect.top_left(), rect.top_right()), Color(0, 255, 0));
		context.fill(Rect::from_corners(rect.top_left(), rect.bot_left()), Color(0, 255, 0));
		context.fill(Rect::from_corners(rect.top_right(), rect.bot_right()), Color(0, 255, 0));
		context.fill(Rect::from_corners(rect.bot_left(), rect.bot_right()), Color(0, 255, 0));
	}
}