#include "shell.hpp"
#include "keyboard.hpp"
#include "lib/string.hpp"
#include "terminal.hpp"

String command;

void execute_command(const StringView& command) {
	if (command.starts_with("echo ")) {
		terminal(command.sub(5));
	}
}

void shell_init() {
	terminal("\n\n$ "_sv);
	KeyboardDispatcher::get().add_listener([](u8 key) {
		if (key == '\n') {
			terminal_put_char('\n');
			execute_command(command);
			command.clear();
			terminal("\n\n$ "_sv);
		} else if (key == 8) {
			if (command.size()) {
				command.pop();
				terminal_delete_char();
			}
		} else {
			command.push_back(key);
			terminal_put_char(key);
		}
	});
}

