#include <kernel/log.hpp>
#include <kernel/window/widget.hpp>

using namespace kernel::window;

void Button::init() {
	const auto text_area = context->calculate_text_area(m_text);
	if (this->rect().size.width == 0) this->resize(Point(text_area.width + 15, rect().size.height));
	if (this->rect().size.height == 0)
		this->resize(Point(rect().size.width, text_area.height + 12));
}

void Button::draw() {
	const auto text_area = context->calculate_text_area(m_text);
	const auto text_pos = (client_rect().bot_right() + Point(1, 1) - text_area) / 2;
	context->fill(client_rect(), active ? Color::from_hex(0x999999) : Color::from_hex(0xBCBCBC));
	context->draw_rect_outline(client_rect(), 1, Color::from_hex(0x4c4c4c));
	context->draw_text(m_text, text_pos, Color(0, 0, 0));
}

void Button::on_mouse_down(Point) {
	active = true;
	invalidate(client_rect());
}

void Button::on_mouse_up(Point) {
	active = false;
	invalidate(client_rect());
}