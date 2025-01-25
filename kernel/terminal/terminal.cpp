#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/tasks/scheduler.hpp>
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
	} else if (str[0] == 'r' && str[1] == ' ') {
		auto hexStr = str.slice(2);
		Vector<u8> data;
		u8 temp = 0;
		int digit = 0;
		for (char c : hexStr) {
			if (c >= '0' && c <= '9') {
				temp += c - '0';
			} else if (c >= 'a' && c <= 'f') {
				temp += c - 'a' + 10;
			} else if (c >= 'A' && c <= 'F') {
				temp += c - 'A' + 10;
			} else {
				continue;
			}
			if (digit == 0) {
				temp *= 16;
				++digit;
			} else {
				digit = 0;
				data.push(temp);
				temp = 0;
			}
		}
		// allocate space for the program
		auto* mem = kernel::alloc::allocate_page();
		memcpy(mem, data.data(), data.size());
		tasks::Scheduler::get().add_process(tasks::allocate_process(mem));
	} else if (str.size()) {
		this->fmt("Unknown command\n");
	}
	this->show_prompt();
}

}