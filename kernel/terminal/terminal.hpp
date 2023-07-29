#pragma once

#include <kernel/window/widget.hpp>
#include <stl/vector.hpp>

namespace kernel::terminal {

class TerminalWindow : public window::Window {
	Vector<char> m_buffer;
	usize m_columns = 0;
	usize m_rows = 0;
	usize m_row = 0, m_column = 0;
	usize m_offset = 0;

	char& char_at(usize col, usize row);
	void invalidate_at(usize col, usize row);

public:
	TerminalWindow(const window::Point& pos, usize cols = 60, usize rows = 20);

	void draw() override;
	void on_key_press(ps2::Key) override;

	void append(char ch);
	void next_line();
};

}