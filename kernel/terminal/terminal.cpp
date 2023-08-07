#include <kernel/log.hpp>
#include <kernel/terminal/terminal.hpp>

namespace kernel::terminal {

Terminal::Terminal(TerminalClient* client) : m_client(client) {}

void Terminal::init() {
	this->show_prompt();
}

void Terminal::show_prompt() {
	m_client->put('$');
	m_client->put(' ');
}

void Terminal::handle_input(StringView str) {
	if (str == "hello"_sv) {
		this->fmt("Hello!\n");
	} else if (str == "bye"_sv) {
		this->fmt("Goodbye!\n");
	} else if (str.size()) {
		this->fmt("Unknown command\n");
	}
	this->show_prompt();
}

}