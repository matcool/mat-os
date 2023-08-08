#pragma once

#include <kernel/window/context.hpp>
#include <kernel/window/widget.hpp>

namespace kernel::window {

// The window manager, which holds all windows
class WindowManager : public Widget {
	WindowContext m_real_context;

	Point m_mouse_pos = Point(0, 0);
	// Last position the mouse was rendered
	Point m_prev_mouse_pos = Point(0, 0);

	u64 m_last_render = 0;

	WindowManager(WindowContext context);

public:
	static WindowManager& get();
	static bool initialized();

	void init() override;

	void handle_mouse(Point off, bool pressed);

	auto width() const { return m_real_context.width(); }

	auto height() const { return m_real_context.height(); }

	void draw() override;
	void draw_mouse();

#if DEBUG_DRAW_RECTS
	void draw_debug(Canvas* canvas);

	bool changed() { return context->drawn_rects; }
#endif
};

}
