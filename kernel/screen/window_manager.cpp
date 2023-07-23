#include <stl/types.hpp>
#include <stl/random.hpp>
#include <stl/math.hpp>
#include <stl/vector.hpp>
#include <kernel/screen/window_manager.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/device/pit.hpp>
#include <kernel/log.hpp>

using namespace kernel::window;

// Cuts out `cut` from `target`, resulting in at most 4 smaller rects.
static Vector<Rect> cut_out_rect(Rect target, const Rect& cut) {
	Vector<Rect> out(4);

	// split by left edge
	if (cut.left() >= target.left() && cut.left() <= target.right()) {
		out.push(Rect::from_corners(target.pos, Point(cut.left() - 1, target.bottom())));
		target.size.width -= cut.left() - target.left();
		target.pos.x = cut.pos.x;
	}

	// split by top edge
	if (cut.top() >= target.top() && cut.top() <= target.bottom()) {
		out.push(Rect::from_corners(target.pos, Point(target.right(), cut.top() - 1)));
		target.size.height -= cut.top() - target.top();
		target.pos.y = cut.pos.y;
	}

	// split by right edge
	if (cut.right() >= target.left() && cut.right() <= target.right()) {
		out.push(Rect::from_corners(Point(cut.right() + 1, target.top()), target.bot_right()));
		target.size.width -= target.right() - cut.right();
	}

	// split by bottom edge
	if (cut.bottom() >= target.top() && cut.bottom() <= target.bottom()) {
		out.push(Rect::from_corners(Point(target.left(), cut.bottom() + 1), target.bot_right()));
		target.size.height -= target.bottom() - cut.bottom();
	}

	return out;
}

void WindowContext::subtract_clip_rect(const Rect& rect) {
	for (usize i = 0; i < clip_rects.size(); ) {
		if (!clip_rects[i].intersects(rect)) {
			++i;
			continue;
		}

		// new rectangle intersects existing one, needs to be split
		const auto target = clip_rects[i];
		clip_rects.remove(i);
		clip_rects.concat(cut_out_rect(target, rect).span());

		// reiterate because of the new rects..
		i = 0;
	}
}

void WindowContext::add_clip_rect(const Rect& rect) {
	// make space for it first
	subtract_clip_rect(rect);
	// then add it
	clip_rects.push(rect);
}

void WindowContext::fill_clipped(Rect rect, const Rect& clip, Color color) {
	if (rect.pos.x < clip.pos.x) {
		rect.size.width -= clip.pos.x - rect.pos.x;
		rect.pos.x = clip.pos.x;
	}
	if (rect.pos.y < clip.pos.y) {
        rect.size.height -= clip.pos.y - rect.pos.y;
        rect.pos.y = clip.pos.y;
    }
	if (rect.right() > clip.right()) {
		rect.size.width -= rect.right() - clip.right();
	}
	if (rect.bottom() > clip.bottom()) {
		rect.size.height -= rect.bottom() - clip.bottom();
	}
	fill_unclipped(rect, color);
}

void WindowContext::fill(const Rect& rect, Color color) {
	if (clip_rects) {
		for (const auto& clip : clip_rects) {
			fill_clipped(rect, clip, color);
		}
	} else {
		fill_unclipped(rect, color);
	}
}

Window::Window(Rect rect) : rect(rect) {
	static random::Generator rng(3);
	fill_color = Color(rng.range(0x00FFFFFF) | 0xFF000000);
}

void Window::paint(WindowContext& context) {
	context.fill(rect, fill_color);
}

void WindowManager::paint() {
	if (!context.data()) return;

	context.clip_rects.clear();

	context.add_clip_rect(Rect(0, 0, width(), height()));

	for (auto& win : children) {
		// win.paint(context);
		context.add_clip_rect(win.rect);
	}

	// draw background
	context.fill(Rect(0, 0, width(), height()), 0);

	for (auto& win : children) {
		win.paint(context);
		// context.add_clip_rect(win.rect);
	}

	for (auto& rect : context.clip_rects) {
		context.fill_unclipped(Rect::from_corners(rect.top_left(), rect.top_right()), Color(255, 255, 0));
		context.fill_unclipped(Rect::from_corners(rect.top_left(), rect.bot_left()), Color(255, 255, 0));

		context.fill_unclipped(Rect::from_corners(rect.bot_left(), rect.bot_right()), Color(200, 200, 0));
		context.fill_unclipped(Rect::from_corners(rect.top_right(), rect.bot_right()), Color(200, 200, 0));
	}

	context.fill_unclipped(Rect(mouse_pos, Point(10, 10)), Color(255, 0, 0));
}

void WindowManager::handle_mouse(Point off, bool pressed) {
	mouse_pos.x = math::clamp(mouse_pos.x + off.x, 0, width());
	mouse_pos.y = math::clamp(mouse_pos.y + off.y, 0, height());

	if (pressed && !last_pressed) {
		usize selected = -1;
		for (usize i = children.size(); i--; ) {
			if (children[i].rect.contains(mouse_pos)) {
				selected = i;
				break;
			}
		}
		if (selected != usize(-1)) {
			// this will copy the window!
			// in the future, windows will probably be behind shared ptrs
			auto window = children[selected];
			children.remove(selected);
			children.push(window);

			dragged_window = &children[children.size() - 1];
			drag_offset = mouse_pos - window.rect.pos;
		}
	} else if (!pressed) {
		dragged_window = nullptr;
	}

	if (dragged_window) {
		dragged_window->rect.pos = mouse_pos - drag_offset;
	}

	last_pressed = pressed;
	
	if (kernel::pit::get_ticks() - last_render > 5) {
		paint();
		last_render = kernel::pit::get_ticks();
	}
}

WindowManager& WindowManager::get() {
	static WindowManager instance;
	return instance;
}

void WindowManager::init() {
	context = kernel::framebuffer::get_framebuffer();

	children.push(Window(Rect(10, 10, 300, 200)));
	children.push(Window(Rect(100, 150, 400, 400)));
	children.push(Window(Rect(200, 100, 200, 600)));
}
