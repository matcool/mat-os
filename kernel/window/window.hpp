#pragma once

#include <stl/pointer.hpp>
#include <kernel/window/context.hpp>

namespace kernel::window {

struct Window;
using WindowPtr = SharedPtr<Window>;

// Represents a window, which is anything that can be on screen.
struct Window {
	// Rect of the *whole* window, including decorations.
	Rect window_rect;
	WindowContext* context = nullptr;

	Window* parent = nullptr;
	Vector<WindowPtr> children;

	bool decoration = true;
	bool last_pressed = false;

	// Window which is taking mouse inputs
	WindowPtr event_child;
	// Top most window
	WindowPtr focus_child;
	WindowPtr drag_child;
	Point drag_offset = Point(0, 0);

	Window(Rect rect);

	void handle_mouse(Point mouse_pos, bool pressed);

	void add_child(WindowPtr window);

	void raise(bool redraw = true);

	void move_to(const Point& pos);

	// Calculate clipping rectangles based on parent's clipping rect.
	void clip_bounds(bool clip_decoration, const Vector<Rect>& dirty_rects = {}) const;

	// Calculates the proper context, then draws this and all children.
	void paint(const Vector<Rect>& dirty_rects = {}, bool paint_children = true);

	// Draws only own window, with the context already set up.
	virtual void draw();

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
	void draw_decoration();

	virtual void on_mouse_down(Point);
};

}