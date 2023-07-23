#pragma once

#include <kernel/screen/window/context.hpp>
#include <kernel/screen/window/window.hpp>

namespace kernel::window {

// The window manager, which holds all windows
class WindowManager : public Window {
	WindowContext context;

	Point mouse_pos = Point(0, 0);
	
	u64 last_render = 0;
	
	WindowManager(WindowContext context);
public:
	static WindowManager& get();

	void init();

	void handle_mouse(Point off, bool pressed);

	auto width() const { return context.width(); }
	auto height() const { return context.height(); }

	void paint();
	void draw(WindowContext& context) override;
};

}

