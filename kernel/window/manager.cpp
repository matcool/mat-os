#include <kernel/window/manager.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/device/pit.hpp>
#include <kernel/log.hpp>
#include <kernel/screen/terminal.hpp>

using namespace kernel::window;

static constexpr Color background_color = Color(100, 100, 200);

void WindowManager::paint() {
	if (!real_context.data()) return;

	terminal::go_to(0, 0);

	Window::paint();

	terminal::fmtln("mouse: {}, {}", mouse_pos.x, mouse_pos.y);
	terminal::fmtln("window: {}, {}", children.last()->window_rect.pos.x, children.last()->window_rect.pos.y);
}

void WindowManager::draw() {
	context->fill(window_rect, background_color);
}

#if DEBUG_DRAW_RECTS
void WindowManager::draw_debug(Canvas* canvas) {
	static constexpr auto blend_colors = [] (Color a, Color b) {
		const i32 a_alpha = a.a ? a.a : 255;
		const i32 b_alpha = b.a ? b.a : 255;
		const auto sum = a_alpha + b_alpha;
		return Color(
            (a.r * a_alpha + b.r * b_alpha) / sum,
            (a.g * a_alpha + b.g * b_alpha) / sum,
            (a.b * a_alpha + b.b * b_alpha) / sum
		);
	};
	
	for (const auto& rect : context->drawn_rects) {
		const auto mid_point = rect.size;
		const auto color = Color(mid_point.x + mid_point.y, mid_point.x * mid_point.y, 0, 200);
		for (i32 j = rect.top(); j <= rect.bottom() && j < canvas->height(); ++j) {
			for (i32 i = rect.left(); i <= rect.right() && i < canvas->width(); ++i) {
				canvas->set(i, j, blend_colors(canvas->get(i, j), color));
			}
		}
	}
	context->drawn_rects.clear();
}
#endif

void WindowManager::handle_mouse(Point off, bool pressed) {
	mouse_pos.x = math::clamp(mouse_pos.x + off.x, 0, width());
	mouse_pos.y = math::clamp(mouse_pos.y + off.y, 0, height());

	if (kernel::pit::get_ticks() - last_render > 2) {
		Window::handle_mouse(mouse_pos, pressed);
		last_render = kernel::pit::get_ticks();
		real_context.fill_unclipped(Rect(mouse_pos, Point(10, 10)), Color(255, 0, 0));
#if DEBUG_DRAW_RECTS
		if (pressed)
			context->drawn_rects.push(Rect(0, 0, 0, 0));
#endif
	}
}

WindowManager::WindowManager(WindowContext context)
	: Window(Rect(0, 0, context.width(), context.height())), real_context(context) {
	decoration = false;
	mouse_pos = window_rect.mid_point();
	// nice
	this->context = &real_context;
}

WindowManager& WindowManager::get() {
	static WindowManager instance(kernel::framebuffer::get_framebuffer());
	return instance;
}

void WindowManager::init() {
	add_child(make_shared<Window>(Rect(20, 20, 300, 200)));
	add_child(make_shared<Window>(Rect(100, 150, 400, 400)));
	add_child(make_shared<Window>(Rect(200, 100, 200, 600)));
}
