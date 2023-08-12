#pragma once

#include <kernel/device/ps2.hpp>
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
	WindowContext* m_context = nullptr;

	Widget* m_parent = nullptr;
	Vector<WidgetPtr> m_children;

	bool m_last_pressed = false;

	// Child which is currently taking mouse inputs
	WidgetPtr m_event_child;
	// Child which is currently focused, which is the last one that was clicked on. If the last mouse
	// click was on none of the children, then this is null, indicating that the focus is on ourselves.
	WidgetPtr m_focus_child;

public:
	Widget(Rect rect);

	void handle_mouse(Point mouse_pos, bool pressed);
	void handle_keyboard(ps2::Key key, bool pressed);

	// Adds a child widget, and calls its init method.
	// TODO: reconsider the init method..
	void add_child(WidgetPtr window);

	// This is called after the widget is added to the parent, and things such as the context are set up.
	virtual void init() {}

	// Moves the widget to a position, and repaints the screen accordingly.
	void move_to(const Point& pos);

	// Resizes the widget rect, and repaints the screen accordingly.
	void resize(const Point& new_size);

	// Gets the index of a child of this widget. -1 is returned if the child was not found.
	usize get_child_index(Widget* child) const;

	// Reorders the given child to be the top most one.
	// TODO: this is a bad name i know
	void reorder_child_top(Widget* child);

	// Returns an iterator of all windows above this one.
	auto iter_windows_above() {
		const auto index = m_parent->get_child_index(this);
		return m_parent->m_children.iter().skip(index + 1);
	}

	// Returns an iterator of all windows below this one.
	auto iter_windows_below() {
		const auto index = m_parent->get_child_index(this);
		return m_parent->m_children.iter().take(index);
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

	// This is triggered when the mouse starts clicking on this widget.
	// Point is relative to this widget's rect.
	virtual void on_mouse_down(Point) {}

	// This is only triggered if the mouse started the click on this widget, and then releases it.
	// Point is relative to this widget's rect.
	virtual void on_mouse_up(Point) {}

	// Point is relative to this widget's rect.
	virtual void on_mouse_move(Point) {}

	// This is triggered when the widget itself or one of its children gains focus.
	virtual void on_focus() {}

	// This is triggered when a key is pressed, and also when its auto-repeating..
	virtual void on_key_press(ps2::Key) {}

	virtual String debug() { return "Widget"_sv; }
};

// Represents a window with decoration such as titlebar, name, and buttons
class Window : public Widget {
	String m_title;
	Point m_drag_offset = Point(0, 0);
	bool m_dragging = false;

public:
	Window(Rect rect, StringView title = "hello world"_sv);

	// Raises the window on the parent, like when focusing.
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

// Represents a simple button with text that can be clicked on.
class Button : public Widget {
	String m_text;

public:
	Button(Rect rect, String text) : Widget(rect), m_text(text) {}

	bool active = false;

	void init() override;
	void draw() override;
	void on_mouse_down(Point) override;
	void on_mouse_up(Point) override;

	String debug() override { return "Button"_sv; }
};

}