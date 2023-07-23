#pragma once

#include <stl/vector.hpp>
#include <stl/math.hpp>
#include <kernel/screen/canvas.hpp>

namespace kernel::window {

using Point = math::Vec2<i32>;
using Rect = math::Rect<i32>;

struct WindowContext : public Canvas {
	Vector<Rect> clip_rects;

	WindowContext() : Canvas(nullptr, 0, 0) {}

	auto& operator=(const Canvas& other) {
		Canvas::operator=(other);
		return *this;
	};

	// Subtracts a rectangle from the clipping rect list.
	void subtract_clip_rect(const Rect& rect);

	// Adds a clipping rectangle, cutting the existing ones if needed.
	void add_clip_rect(const Rect& rect);

	void clear_clip_rects();

	// Draws a rectangle clipped by `clip`.
	void fill_clipped(Rect rect, const Rect& clip, Color color);

	// Draws a rectangle, taking into account the clipping rects.
	void fill(const Rect& rect, Color color);

	// Draws a rectangle directly.
	void fill_unclipped(const Rect& rect, Color color) {
		Canvas::fill(rect, color);
	}
};

// Represents a *simple* window, which is just a rect
// with solid color, for now.
struct Window {
	Rect rect;

	Window(Rect rect);

	void paint(WindowContext&);
};

// The window manager, which holds all windows, and does other
// calculations such as clipping.
struct WindowManager {
	Vector<Window> children;
	WindowContext context;

	Point mouse_pos = Point(0, 0);
	bool last_pressed = false;

	Window* dragged_window = nullptr;
	Point drag_offset = Point(0, 0);

	
	u64 last_render = 0;
	
	static WindowManager& get();

	void init();

	// Handles mouse movement.
	void handle_mouse(Point off, bool pressed);

	auto width() const { return context.width(); }
	auto height() const { return context.height(); }

	// Renders everything onto the screen.
	void paint();
};

}

