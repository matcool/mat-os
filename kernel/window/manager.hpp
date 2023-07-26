#pragma once

#include <kernel/window/context.hpp>
#include <kernel/window/window.hpp>

namespace kernel::window {

// The window manager, which holds all windows
class WindowManager : public Window {
	WindowContext real_context;

	Point mouse_pos = Point(0, 0);
	// Last position the mouse was rendered
	Point prev_mouse_pos = Point(0, 0);

	u64 last_render = 0;

	WindowManager(WindowContext context);

public:
	static WindowManager& get();

	void init();

	void handle_mouse(Point off, bool pressed);

	auto width() const { return real_context.width(); }

	auto height() const { return real_context.height(); }

	void draw() override;
	void draw_mouse();

#if DEBUG_DRAW_RECTS
	void draw_debug(Canvas* canvas);

	bool changed() { return context->drawn_rects; }
#endif
};

}
