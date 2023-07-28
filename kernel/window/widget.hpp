#pragma once

#include <kernel/window/context.hpp>
#include <stl/iterator.hpp>
#include <stl/pointer.hpp>
#include <stl/span.hpp>
#include <stl/string.hpp>

namespace kernel::window {

class Widget;
using WidgetPtr = SharedPtr<Widget>;

// Represents a widget, which is anything that can be on screen.
class Widget {
	// Rect of the *whole* widget
	Rect m_rect;

protected:
	WindowContext* context = nullptr;

	Widget* parent = nullptr;
	Vector<WidgetPtr> children;

	bool last_pressed = false;

	// Child which is currently taking mouse inputs
	WidgetPtr event_child;

public:
	Widget(Rect rect);

	void handle_mouse(Point mouse_pos, bool pressed);

	void add_child(WidgetPtr window);

	void move_to(const Point& pos);

	void resize(const Point& new_size);

	usize get_child_index(Widget* child) const;

	// Reorders the given child to be the top most one.
	// this is a bad name i know
	void reorder_child_top(Widget* child);

	// Returns an iterator of all windows above this one.
	auto iter_windows_above() {
		const auto index = parent->get_child_index(this);
		return parent->children.iter().skip(index + 1);
	}

	// Returns an iterator of all windows below this one.
	auto iter_windows_below() {
		const auto index = parent->get_child_index(this);
		return parent->children.iter().take(index);
	}

	// Invalidates a portion of the window, so it can be painted onto the screen.
	// The rect is relative to the widget's rect.
	void invalidate(const Rect& rect);

	// Calculate clipping rectangles based on parent's clipping rect.
	void clip_bounds(bool clip_decoration, Span<const Rect> dirty_rects = {});

	// Calculates the proper context, then draws this and all children.
	void paint(Span<const Rect> dirty_rects = {}, bool paint_children = true);

	// Rect of this widget, with the position relative to the parent.
	Rect rect() const;

	// `rect()` but relative positioning to the *whole* screen.
	Rect screen_rect() const;

	// Rect for the inside of the widget, excluding decorations. The client rect is where children
	// of this widget will be placed and clipped to. This has no position offset, so its useful to drawing functions.
	Rect client_rect() const;

	// Same as client_rect(), but position is *relative* to the widget rect. For widgets with no
	// decorations, this should be the same as `rect().reset_pos()`.
	virtual Rect relative_client_rect() const;

	// Similar to `screen_rect()`, but for the client rect.
	Rect screen_client_rect() const;

	// Draws only own window, with the context already set up.
	virtual void draw();

	// Draws decoration, such as window borders and title bar. The decorations are drawn first, and are above children.
	virtual void draw_decoration();

	virtual void on_mouse_down(Point);
	virtual void on_mouse_up(Point);
	virtual void on_mouse_move(Point);
	virtual void on_focus();

	virtual String debug() { return "Widget"_sv; }
};

struct Window : public Widget {
	String title;
	Point drag_offset = Point(0, 0);
	bool dragging = false;

	Window(Rect rect, StringView title = "hello world"_sv) : Widget(rect), title(title) {}

	void raise(bool redraw = true);

	void draw() override;
	void draw_decoration() override;
	Rect relative_client_rect() const override;
	void on_mouse_down(Point) override;
	void on_mouse_up(Point) override;
	void on_mouse_move(Point) override;
	void on_focus() override;

	String debug() override { return "Window"_sv; }
};

struct Button : public Widget {
	String m_text;

	Button(Rect rect, String text) : Widget(rect), m_text(text) {}

	bool active = false;

	void draw() override;
	void on_mouse_down(Point) override;
	void on_mouse_up(Point) override;

	String debug() override { return "Button"_sv; }
};

}