#pragma once

#include <stl/types.hpp>
#include <stl/math.hpp>
#include <stl/vector.hpp>
#include <kernel/screen/canvas.hpp>

namespace kernel::window {

using Point = math::Vec2<i32>;
using Rect = math::Rect<i32>;

class WindowContext : public Canvas {
	Vector<Rect> clip_rects;
	Point offset = Point(0, 0);
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
	void fill_clipped(Rect rect, const Rect& clip, Color color);

	// Draws a rectangle, taking into account the clipping rects.
	void fill(const Rect& rect, Color color);

	// Draws a rectangle directly.
	void fill_unclipped(const Rect& rect, Color color) {
		Canvas::fill(rect, color);
	}

	void set_offset(const Point& point) {
		offset = point;
	}
};

}