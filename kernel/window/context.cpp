#include <kernel/window/context.hpp>

using namespace kernel::window;

// Cuts out `cut` from `target`, resulting in at most 4 smaller rects.
static void cut_out_rect(Vector<Rect>& out, Rect target, const Rect& cut) {
	// split by left edge
	if (cut.left() >= target.left() && cut.left() <= target.right()) {
		out.push(Rect::from_corners(target.pos, Point(cut.left() - 1, target.bottom())));
		target.size.width -= cut.left() - target.left();
		target.pos.x = cut.pos.x;
	}

	// split by top edge
	if (cut.top() >= target.top() && cut.top() <= target.bottom()) {
		out.push(Rect::from_corners(target.pos, Point(target.right(), cut.top() - 1)));
		target.size.height -= cut.top() - target.top();
		target.pos.y = cut.pos.y;
	}

	// split by right edge
	if (cut.right() >= target.left() && cut.right() <= target.right()) {
		out.push(Rect::from_corners(Point(cut.right() + 1, target.top()), target.bot_right()));
		target.size.width -= target.right() - cut.right();
	}

	// split by bottom edge
	if (cut.bottom() >= target.top() && cut.bottom() <= target.bottom()) {
		out.push(Rect::from_corners(Point(target.left(), cut.bottom() + 1), target.bot_right()));
		target.size.height -= target.bottom() - cut.bottom();
	}
}

void WindowContext::subtract_clip_rect(const Rect& rect) {
	for (usize i = 0; i < clip_rects.size(); ) {
		if (!clip_rects[i].intersects(rect)) {
			++i;
			continue;
		}

		// new rectangle intersects existing one, needs to be split
		const auto target = clip_rects[i];
		clip_rects.remove(i);
		cut_out_rect(clip_rects, target, rect);

		// reiterate because of the new rects..
		i = 0;
	}
}

void WindowContext::add_clip_rect(const Rect& rect) {
	// make space for it first
	subtract_clip_rect(rect);
	// then add it
	clip_rects.push(rect);
}

void WindowContext::intersect_clip_rect(const Rect& clip) {
	Vector<Rect> new_rects;

	for (const auto& rect : clip_rects) {
		const auto intersection = rect.intersection(clip);
		if (!intersection.empty()) {
			new_rects.push(intersection);
		}
	}

	clip_rects = new_rects;
}

void WindowContext::clear_clip_rects() {
	clip_rects.clear();
}

void WindowContext::fill_clipped(Rect rect, const Rect& clip, Color color) {
	rect.pos += offset;

	if (rect.pos.x < clip.pos.x) {
		rect.size.width -= clip.pos.x - rect.pos.x;
		rect.pos.x = clip.pos.x;
	}
	if (rect.pos.y < clip.pos.y) {
        rect.size.height -= clip.pos.y - rect.pos.y;
        rect.pos.y = clip.pos.y;
    }
	if (rect.right() > clip.right()) {
		rect.size.width -= rect.right() - clip.right();
	}
	if (rect.bottom() > clip.bottom()) {
		rect.size.height -= rect.bottom() - clip.bottom();
	}
	fill_unclipped(rect, color);
}

void WindowContext::fill(const Rect& rect, Color color) {
	if (clip_rects) {
		for (const auto& clip : clip_rects) {
			fill_clipped(rect, clip, color);
		}
	} else {
		fill_unclipped(rect + offset, color);
	}
}

void WindowContext::draw_rect_outline(const Rect& rect, i32 width, Color color) {
	const auto off_x = Point(width, 0);
	const auto off_y = Point(0, width);
	fill(Rect::from_corners(rect.top_left(), rect.top_right() + off_y), color);
	fill(Rect::from_corners(rect.bot_left() - off_y, rect.bot_right()), color);

	fill(Rect::from_corners(rect.top_left(), rect.bot_left() + off_x), color);
	fill(Rect::from_corners(rect.top_right() - off_x, rect.bot_right()), color);
}