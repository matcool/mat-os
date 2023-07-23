#include <kernel/screen/window/manager.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/device/pit.hpp>
#include <kernel/log.hpp>

using namespace kernel::window;

static constexpr Color background_color = Color(100, 100, 200);

void WindowManager::paint() {
	if (!context.data()) return;

	context.clear_clip_rects();

	const auto desktop_rect = Rect(0, 0, width(), height());

	context.add_clip_rect(desktop_rect);

	for (auto& win : children) {
		context.subtract_clip_rect(win.rect);
	}

	context.fill(desktop_rect, background_color);

	context.clear_clip_rects();

	for (usize i = 0; i < children.size(); ++i) {
		auto& win = children[i];
		context.add_clip_rect(win.rect);

		// iterate through windows above this one
		for (usize j = i + 1; j < children.size(); ++j) {
			// if it doesnt intersect the window, ignore it
			if (!win.rect.intersects(children[j].rect))
				continue;
			
			context.subtract_clip_rect(children[j].rect);
		}

		// should be clipped properly
		win.paint(context);

		context.clear_clip_rects();
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
