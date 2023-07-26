#include <kernel/device/pit.hpp>
#include <kernel/log.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/window/manager.hpp>
#include <kernel/window/theme.hpp>

using namespace kernel::window;

void WindowManager::draw() {
	context->fill(window_rect, theme::DESKTOP_COLOR);
}

#if DEBUG_DRAW_RECTS
void WindowManager::draw_debug(Canvas* canvas) {
	static constexpr auto blend_colors = [](Color a, Color b) {
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

	// mouse interrupts happen way too often, and end up lagging
	// window dragging even with our optimized system, so limit it
	// to every 2ms (500fps)
	if (kernel::pit::get_ticks() - last_render > 2) {
		Window::handle_mouse(mouse_pos, pressed);
		last_render = kernel::pit::get_ticks();
		draw_mouse();
		prev_mouse_pos = mouse_pos;
#if DEBUG_DRAW_RECTS
		if (pressed) context->drawn_rects.push(Rect(0, 0, 0, 0));
#endif
	}
}

constexpr u32 EMPTY = 0x0;
constexpr u32 BLACK = 0xFF000000;
constexpr u32 WHITE = 0xFFFFFFFF;

constexpr usize MOUSE_WIDTH = 9;
constexpr usize MOUSE_HEIGHT = 13;

// clang-format off
u32 mouse_sprite[MOUSE_WIDTH * MOUSE_HEIGHT] = {
	BLACK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
	BLACK, BLACK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
	BLACK, WHITE, BLACK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
	BLACK, WHITE, WHITE, BLACK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
	BLACK, WHITE, WHITE, WHITE, BLACK, EMPTY, EMPTY, EMPTY, EMPTY,
	BLACK, WHITE, WHITE, WHITE, WHITE, BLACK, EMPTY, EMPTY, EMPTY,
	BLACK, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, EMPTY, EMPTY,
	BLACK, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, EMPTY,
	BLACK, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK,
	BLACK, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, EMPTY, EMPTY,
	BLACK, WHITE, BLACK, BLACK, WHITE, BLACK, EMPTY, EMPTY, EMPTY,
	BLACK, BLACK, EMPTY, EMPTY, BLACK, WHITE, BLACK, EMPTY, EMPTY,
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, BLACK, EMPTY, EMPTY, EMPTY,
};

// clang-format on

void WindowManager::draw_mouse() {
	Canvas mouse_canvas(mouse_sprite, MOUSE_WIDTH, MOUSE_HEIGHT);
	const auto old_mouse_rect = Rect(prev_mouse_pos, Point(MOUSE_WIDTH, MOUSE_HEIGHT));
	Window::paint(Span(&old_mouse_rect, 1), true);
	context->paste_alpha_masked(mouse_canvas, mouse_pos.x, mouse_pos.y);
}

WindowManager::WindowManager(WindowContext context) :
	Window(Rect(0, 0, context.width(), context.height())), real_context(context) {
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
	children[0]->add_child(make_shared<Button>(Rect(50, 50, 20, 20)));
	children[1]->add_child(make_shared<Window>(Rect(50, 50, 100, 100)));
}
