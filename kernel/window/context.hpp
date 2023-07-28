#pragma once

#include <kernel/screen/canvas.hpp>
#include <stl/math.hpp>
#include <stl/string.hpp>
#include <stl/types.hpp>
#include <stl/vector.hpp>

#define DEBUG_DRAW_RECTS 0

namespace kernel::window {

using Point = math::Vec2<i32>;
using Rect = math::Rect<i32>;

class WindowContext : public Canvas {
	Vector<Rect> clip_rects;
	Point offset = Point(0, 0);
	bool should_clip = false;

public:
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
	void fill_clipped(const Rect& rect, const Rect& clip, Color color);

	// Draws a rectangle, taking into account the clipping rects.
	void fill(const Rect& rect, Color color);

	// Draws a rectangle directly.
	void fill_unclipped(const Rect& rect, Color color) { Canvas::fill(rect, color); }

	// Draws only the outline of the rectangle. Width goes inwards.
	void draw_rect_outline(const Rect& rect, i32 width, Color color);

	// Draws a single character clipped by `clip`.
	void draw_char_clipped(char ch, const Point& pos, const Rect& clip, Color color);

	// Draws a single character, taking into account the clipping rects.
	void draw_char(char ch, const Point& pos, Color color);

	// Draws some text, taking into account the clipping rects.
	// Returns a rect of the area the text occupies.
	Rect draw_text(StringView str, const Point& pos, Color color);

	// Calculates the area the text would take up.
	Point calculate_text_area(StringView str);

	void set_offset(const Point& point) { offset = point; }

	auto get_clip_rects() const { return clip_rects; }

#if DEBUG_DRAW_RECTS
	Vector<Rect> drawn_rects;
#endif
};

}