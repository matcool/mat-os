#include <kernel/font.hpp>
#include <kernel/log.hpp>
#include <kernel/window/context.hpp>
#include <stl/iterator.hpp>

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
	should_clip = true;
	for (usize i = 0; i < clip_rects.size();) {
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
	should_clip = true;
	Vector<Rect> new_rects;

	for (const auto& rect : clip_rects) {
		const auto intersection = rect.intersection(clip);
		if (!intersection.empty()) {
			new_rects.push(intersection);
		}
	}

	clip_rects = new_rects;

	// this is noticeably slower for some reason
	// clip_rects = clip_rects.iter().filter([&](const auto& rect) { return
	// !rect.intersection(clip).empty(); }).collect_vec();
}

void WindowContext::clear_clip_rects() {
	should_clip = false;
	clip_rects.clear();
}

void WindowContext::fill_clipped(const Rect& rect, const Rect& clip, Color color) {
	const auto clipped_rect = (rect + offset).intersection(clip);
	if (clipped_rect.empty()) return;

#if DEBUG_DRAW_RECTS
	drawn_rects.push(clipped_rect);
#endif

	fill_unclipped(clipped_rect, color);
}

void WindowContext::fill(const Rect& rect, Color color) {
	if (clip_rects) {
		for (const auto& clip : clip_rects) {
			fill_clipped(rect, clip, color);
		}
	} else if (!should_clip) {
		fill_unclipped(rect + offset, color);
	}
}

void WindowContext::draw_rect_outline(const Rect& rect, i32 width, Color color) {
	const auto off_x = Point(width - 1, 0);
	const auto off_y = Point(0, width - 1);
	fill(Rect::from_corners(rect.top_left(), rect.top_right() + off_y), color);
	fill(Rect::from_corners(rect.bot_left() - off_y, rect.bot_right()), color);

	fill(Rect::from_corners(rect.top_left(), rect.bot_left() + off_x), color);
	fill(Rect::from_corners(rect.top_right() - off_x, rect.bot_right()), color);
}

void WindowContext::draw_char_clipped(char ch, const Point& pos, const Rect& clip, Color color) {
	const auto char_rect =
		Rect(Point(0, 0), Point(PIXEL_FONT_WIDTH, PIXEL_FONT_HEIGHT)).intersection(clip - offset - pos);
	if (char_rect.empty()) return;

	for (auto y : iterators::range(char_rect.top(), char_rect.bottom() + 1)) {
		const auto row = PIXEL_FONT[static_cast<usize>(ch)][y];
		for (auto x : iterators::range(char_rect.left(), char_rect.right() + 1)) {
			if (math::get_bit(row, x)) this->set(x + offset.x + pos.x, y + offset.y + pos.y, color);
		}
	}
}

void WindowContext::draw_char(char ch, const Point& pos, Color color) {
	if (clip_rects) {
		for (const auto& clip : clip_rects) {
			draw_char_clipped(ch, pos, clip, color);
		}
	} else if (!should_clip) {
		draw_char_clipped(ch, pos, Rect(0, 0, width(), height()), color);
	}
}

Rect WindowContext::draw_text(StringView str, const Point& pos, Color color) {
	Point offset(0, 0);
	for (char c : str) {
		draw_char(c, pos + offset, color);
		offset.x += 7;
	}
	return Rect(pos, Point(offset.x, PIXEL_FONT_HEIGHT));
}

Point WindowContext::calculate_text_area(StringView str) {
	return Point(str.size() * PIXEL_FONT_WIDTH, PIXEL_FONT_HEIGHT);
}