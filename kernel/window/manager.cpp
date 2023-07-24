#include <kernel/window/manager.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/device/pit.hpp>
#include <kernel/log.hpp>
#include <kernel/screen/terminal.hpp>

using namespace kernel::window;

static constexpr Color background_color = Color(100, 100, 200);

void WindowManager::paint() {
	if (!context.data()) return;

	terminal::go_to(0, 0);

	Window::paint(context);

	terminal::fmtln("mouse: {}, {}", mouse_pos.x, mouse_pos.y);
	terminal::fmtln("window: {}, {}", children.last()->window_rect.pos.x, children.last()->window_rect.pos.y);
}

void WindowManager::draw(WindowContext&) {
	context.fill(window_rect, background_color);
}

void WindowManager::handle_mouse(Point off, bool pressed) {
	mouse_pos.x = math::clamp(mouse_pos.x + off.x, 0, width());
	mouse_pos.y = math::clamp(mouse_pos.y + off.y, 0, height());

	Window::handle_mouse(mouse_pos, pressed);
	
	context.fill_unclipped(Rect(mouse_pos, Point(10, 10)), Color(255, 0, 0));
}

WindowManager::WindowManager(WindowContext context)
	: Window(Rect(0, 0, context.width(), context.height())), context(context) {
	decoration = false;
	mouse_pos = window_rect.mid_point();
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
