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

// Represents a window, which is anything that can be on screen.
struct Window {
	// Rect of the *whole* window, including decorations.
	Rect window_rect;

	Window* parent = nullptr;
	Vector<WindowPtr> children;

	bool decoration = true;
	bool last_pressed = false;

	// Window which is taking mouse inputs
	WindowPtr active_child;
	WindowPtr drag_child;
	Point drag_offset = Point(0, 0);

	Window(Rect rect);

	void handle_mouse(Point mouse_pos, bool pressed);

	void add_child(WindowPtr window);

	// Calculate clipping rectangles based on parent's clipping rect.
	void clip_bounds(WindowContext& context, bool clip_decoration = false) const;

	// Calculates the proper context, then draws this and all children.
	void paint(WindowContext& context);

	// Draws only own window, with the context already set up.
	virtual void draw(WindowContext& context);

	// `window_rect` but relative to the *whole* screen.
	Rect screen_window_rect() const;

	// Rect for the inside of the window, excluding decorations.
	// This has no position offset, so its useful to drawing functions.
	Rect client_rect() const;

	// Same as client_rect(), but position is *relative* to parent.
	// On windows with no decoration this is the same as window_rect,
	// however with decorations this will be window_rect + decoration_offset().
	Rect relative_client_rect() const;

	// Similar to `screen_window_rect`, but will offset if the window has decorations.
	Rect screen_client_rect() const;

	// Rect of the title bar, relative to `window_rect`.
	Rect titlebar_rect() const;

	// Draws decoration, such as window borders and title bar.
	void draw_decoration(WindowContext& context);

	virtual void on_mouse_down(Point);
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

