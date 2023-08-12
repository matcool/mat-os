#include "kernel/window/qoi.hpp"
#include <kernel/device/pit.hpp>
#include <kernel/log.hpp>
#include <kernel/modules.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/terminal/window.hpp>
#include <kernel/window/manager.hpp>
#include <kernel/window/theme.hpp>

using namespace kernel::window;

void WindowManager::draw() {
	m_context->fill(this->client_rect(), theme::DESKTOP_COLOR);
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
	m_mouse_pos.x = math::clamp(m_mouse_pos.x + off.x, 0, width());
	m_mouse_pos.y = math::clamp(m_mouse_pos.y + off.y, 0, height());

	// mouse interrupts happen way too often, and end up lagging
	// window dragging even with our optimized system, so limit it
	// to every 2ms (500fps)
	if (kernel::pit::get_ticks() - m_last_render > 2) {
		Widget::handle_mouse(m_mouse_pos, pressed);
		m_last_render = kernel::pit::get_ticks();
		this->draw_mouse();
		m_prev_mouse_pos = m_mouse_pos;
#if DEBUG_DRAW_RECTS
		if (pressed) context->drawn_rects.push(Rect(0, 0, 0, 0));
#endif
	}
}

void WindowManager::draw_mouse() {
	const auto old_mouse_rect =
		Rect(m_prev_mouse_pos, Point(m_mouse_canvas.width(), m_mouse_canvas.height()));
	Widget::paint(Span(&old_mouse_rect, 1), true);
	m_context->paste_alpha_masked(m_mouse_canvas, m_mouse_pos.x, m_mouse_pos.y);
}

BitmapFont get_default_font() {
	return BitmapFont::from_qoi(kernel::Modules::get().with_path("/assets/font.qoi").data);
}

WindowManager::WindowManager(WindowContext context) :
	Widget(Rect(0, 0, context.width(), context.height())), m_real_context(context),
	m_font(get_default_font()) {
	m_mouse_pos = this->rect().mid_point();
	m_context = &m_real_context;

	QOIStreamDecoder decoder(Modules::get().with_path("/assets/mouse.qoi").data);

	m_mouse_data.reserve(decoder.width() * decoder.height());

	while (!decoder.finished()) {
		m_mouse_data.push(decoder.next_pixel());
	}

	m_mouse_canvas =
		Canvas(reinterpret_cast<u32*>(m_mouse_data.data()), decoder.width(), decoder.height());
}

bool manager_initialized = false;

bool WindowManager::initialized() {
	return manager_initialized;
}

WindowManager& WindowManager::get() {
	static WindowManager instance(kernel::framebuffer::get_framebuffer());
	manager_initialized = true;
	return instance;
}

void WindowManager::init() {
	this->add_child(make_shared<Window>(Rect(20, 20, 300, 200), "with button"_sv));
	this->add_child(make_shared<Window>(Rect(100, 150, 400, 400), "with child"_sv));
	this->add_child(make_shared<Window>(Rect(200, 100, 200, 600), "long"_sv));
	m_children[0]->add_child(make_shared<Button>(Rect(50, 50, 0, 0), "Hello"_sv));
	m_children[0]->add_child(make_shared<Button>(Rect(50, 80, 0, 0), "World!"_sv));
	m_children[1]->add_child(make_shared<Window>(Rect(50, 50, 100, 100), "inner"_sv));

	this->add_child(make_shared<terminal::TerminalWindow>(Point(100, 100)));
}
