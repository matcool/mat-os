#pragma once

#include <kernel/terminal/terminal.hpp>
#include <kernel/window/widget.hpp>
#include <stl/string.hpp>
#include <stl/vector.hpp>

namespace kernel::terminal {

class TerminalWindow : public window::Window, TerminalClient {
	Vector<char> m_buffer;
	usize m_columns = 0;
	usize m_rows = 0;
	usize m_row = 0, m_column = 0;
	usize m_offset = 0;
	Terminal m_terminal;
	String m_input;

	char& char_at(usize col, usize row);
	void invalidate_at(usize col, usize row);

	void init() override;

public:
	void put(char ch) override;
	TerminalWindow(const window::Point& pos, usize cols = 60, usize rows = 20);

	void draw() override;
	void on_key_press(ps2::Key) override;

	void append(char ch);
	void next_line();
};

}