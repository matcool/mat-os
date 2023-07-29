#include <kernel/font.hpp>
#include <kernel/log.hpp>
#include <kernel/terminal/terminal.hpp>

using namespace kernel::terminal;
using kernel::window::Point;
using kernel::window::Rect;

TerminalWindow::TerminalWindow(const Point& pos, usize cols, usize rows) :
	Window(Rect(pos, Point(cols * PIXEL_FONT_WIDTH, rows * PIXEL_FONT_HEIGHT)), "terminal"_sv),
	m_columns(cols), m_rows(rows) {
	m_buffer.reserve(cols * rows);
	for (usize i = 0; i < cols * rows; ++i) {
		m_buffer.push('\0');
	}
}

char& TerminalWindow::char_at(usize col, usize row) {
	return m_buffer[(m_offset + row * m_columns + col) % m_buffer.size()];
}

void TerminalWindow::draw() {
	context->fill(client_rect(), Color(0, 0, 0));
	for (usize j = 0; j < m_rows; ++j) {
		for (usize i = 0; i < m_columns; ++i) {
			const char ch = this->char_at(i, j);
			if (ch == '\0') continue;
			context->draw_char(
				ch, Point(i * PIXEL_FONT_WIDTH, j * PIXEL_FONT_HEIGHT), Color(255, 255, 255)
			);
		}
	}
}

void TerminalWindow::on_key_press(ps2::Key key) {
	if (key.ch) this->append(key.ch);
}

void TerminalWindow::append(char ch) {
	if (ch == '\0') return;
	if (ch == '\n') return this->next_line();
	// backspace
	if (ch == '\x08') {
		if (m_column == 0 && m_row) {
			m_column = m_columns;
			--m_row;
		}
		if (m_column) --m_column;
		char_at(m_column, m_row) = '\0';
		invalidate_at(m_column, m_row);
		return;
	}
	if (m_column == m_columns) this->next_line();

	char_at(m_column, m_row) = ch;
	invalidate_at(m_column, m_row);
	m_column++;
}

void TerminalWindow::invalidate_at(usize col, usize row) {
	this->invalidate(
		Rect(col * PIXEL_FONT_WIDTH, row * PIXEL_FONT_HEIGHT, PIXEL_FONT_WIDTH, PIXEL_FONT_HEIGHT) +
		relative_client_rect().pos
	);
}

void TerminalWindow::next_line() {
	m_column = 0;
	if (m_row == m_rows - 1) {
		for (usize i = 0; i < m_columns; ++i) {
			char_at(i, 0) = '\0';
		}
		m_offset += m_columns;
		this->invalidate(relative_client_rect());
	} else {
		m_row++;
	}
}