#include <stl/types.hpp>
#include <stl/random.hpp>
#include <stl/math.hpp>
#include <stl/vector.hpp>
#include <kernel/screen/windows.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/device/pit.hpp>
#include <kernel/log.hpp>

using Point = math::Vec2<i32>;
using Rect = math::Rect<i32>;

// Cuts out `cut` from `target`, resulting in at most 4 smaller rects.
Vector<Rect> split_rect(Rect target, const Rect& cut) {
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

struct Window {
	Rect rect;
	Color fill_color;

	Window(Rect rect) : rect(rect) {
		static random::Generator rng(3);
		fill_color = Color(rng.range(0x00FFFFFF) | 0xFF000000);
	}

	void paint(Canvas* context) {
		context->fill(rect.pos.x, rect.pos.y, rect.size.width, rect.size.height, fill_color);
	}
};

struct Desktop {
	Vector<Window> children;
	Canvas* context = nullptr;
	Point mouse_pos = Point(0, 0);
	bool last_pressed = false;
	Window* dragged_window = nullptr;
	Point drag_offset = Point(0, 0);

	Vector<Rect> clip_rects;
	
	u64 last_render = 0;

	void paint() {
		clip_rects.clear();

		// draw background
		context->fill(0, 0, context->width(), context->height(), 0xFF3399FF);

		for (auto& win : children) {
			win.paint(context);
			add_clip_rect(win.rect);
		}

		for (auto& rect : clip_rects) {
			context->fill(rect.pos.x, rect.pos.y, rect.size.width, 1, Color(255, 255, 0));
			context->fill(rect.pos.x, rect.pos.y, 1, rect.size.height, Color(255, 255, 0));
			context->fill(rect.right(), rect.pos.y, 1, rect.size.height, Color(200, 200, 0));
			context->fill(rect.pos.x, rect.bottom(), rect.size.width, 1, Color(200, 200, 0));
		}

		context->fill(mouse_pos.x, mouse_pos.y, 10, 10, Color(0, 0, 0));
	}

	auto width() const { return context->width(); }
	auto height() const { return context->height(); }

	void handle_mouse(Point off, bool pressed) {
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
		
		if (kernel::pit::get_ticks() - last_render > 16) {
			paint();
			last_render = kernel::pit::get_ticks();
		}
	}

	void add_clip_rect(const Rect& rect) {
		for (usize i = 0; i < clip_rects.size(); ) {
			if (!clip_rects[i].intersects(rect)) {
				++i;
				continue;
			}

			// new rectangle intersects existing one, needs to be split
			const auto target = clip_rects[i];
			clip_rects.remove(i);
			clip_rects.concat(split_rect(target, rect).span());

			// reiterate because of the new rects..
			i = 0;
		}
		clip_rects.push(rect);
	}
};

static auto& get_desktop() {
	static Desktop desktop;
	return desktop;
}

void draw_windows() {
	auto* fb = &kernel::framebuffer::get_framebuffer();

	auto& desktop = get_desktop();
	desktop.context = fb;

	desktop.children.push(Window(Rect(10, 10, 300, 200)));
	desktop.children.push(Window(Rect(100, 150, 400, 400)));
	desktop.children.push(Window(Rect(200, 100, 200, 600)));

	desktop.paint();
}

void handle_mouse_movement(i32 off_x, i32 off_y, bool pressed) {
	get_desktop().handle_mouse(Point(off_x, off_y), pressed);
}