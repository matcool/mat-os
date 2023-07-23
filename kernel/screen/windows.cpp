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
	
	u64 last_render = 0;

	void paint() {
		// draw background
		context->fill(0, 0, context->width(), context->height(), 0xFF3399FF);

		for (auto& win : children) {
			win.paint(context);
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
};

static auto& get_desktop() {
	static Desktop desktop;
	return desktop;
}

void draw_windows() {
	auto* fb = kernel::framebuffer::get_framebuffer();

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