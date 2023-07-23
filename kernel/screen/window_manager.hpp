#pragma once

#include <stl/vector.hpp>
#include <stl/math.hpp>
#include <kernel/screen/canvas.hpp>

namespace kernel::window {

using Point = math::Vec2<i32>;
using Rect = math::Rect<i32>;

// Represents a *simple* window, which is just a rect
// with solid color, for now.
struct Window {
	Rect rect;
	Color fill_color;

	Window(Rect rect);

	void paint(Canvas*);
};

// The window manager, which holds all windows, and does other
// calculations such as clipping.
struct WindowManager {
	Vector<Window> children;
	Canvas* context = nullptr;

	Point mouse_pos = Point(0, 0);
	bool last_pressed = false;

	Window* dragged_window = nullptr;
	Point drag_offset = Point(0, 0);

	Vector<Rect> clip_rects;
	
	u64 last_render = 0;
	
	static WindowManager& get();

	void init();

	// Handles mouse movement.
	void handle_mouse(Point off, bool pressed);

	auto width() const { return context->width(); }
	auto height() const { return context->height(); }

	// Renders everything onto the screen.
	void paint();

	// Adds a clipping rectangle, cutting the existing ones if needed.
	void add_clip_rect(const Rect& rect);
};

}

