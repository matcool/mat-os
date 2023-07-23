#pragma once

#include <stl/vector.hpp>
#include <stl/math.hpp>
#include <stl/pointer.hpp>
#include <kernel/screen/canvas.hpp>

namespace kernel::window {

using Point = math::Vec2<i32>;
using Rect = math::Rect<i32>;

struct WindowContext : public Canvas {
	Vector<Rect> clip_rects;
	Point offset = Point(0, 0);

	WindowContext(const Canvas& canvas) : Canvas(canvas) {}

	// Subtracts a rectangle from the clipping rect list.
	void subtract_clip_rect(const Rect& rect);

	// Adds a clipping rectangle, cutting the existing ones if needed.
	void add_clip_rect(const Rect& rect);
	
	// Intersects the clipping rect list.
	void intersect_clip_rect(const Rect& rect);

	// Clears the clipping rect list.
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

struct Window;
class WindowManager;
using WindowPtr = SharedPtr<Window>;

// Represents a *simple* window, which is just a rect
// with solid color, for now.
struct Window {
	Rect rect;

	Window* parent = nullptr;
	Vector<WindowPtr> children;

	bool draggable = true;
	bool decoration = true;
	bool last_pressed = false;

	WindowPtr drag_child;
	Point drag_offset = Point(0, 0);

	Window(Rect rect);

	void handle_mouse(Point mouse_pos, bool pressed);

	void add_child(WindowPtr window);

	// Parent's screen pos. If there is no parent, return (0, 0)
	Point parent_screen_pos() const;
	Point screen_pos() const;
	Rect screen_rect() const { return Rect(screen_pos(), rect.size); }

	// Calculate clipping rectangles based on parent's clipping rect.
	void clip_bounds(WindowContext& context) const;

	// Calculates the proper context, then draws this and all children.
	void paint(WindowContext& context);

	// Draws only own window, with the context already set up.
	virtual void draw(WindowContext& context);
};

// The window manager, which holds all windows
class WindowManager : public Window {
	WindowContext context;

	Point mouse_pos = Point(0, 0);
	
	u64 last_render = 0;
	
	WindowManager(WindowContext context);
public:
	static WindowManager& get();

	void init();

	void handle_mouse(Point off, bool pressed);

	auto width() const { return context.width(); }
	auto height() const { return context.height(); }

	void paint();
	void draw(WindowContext& context) override;
};

}

