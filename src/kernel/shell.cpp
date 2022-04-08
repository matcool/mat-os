#include "shell.hpp"
#include "keyboard.hpp"
#include "lib/string.hpp"
#include "terminal.hpp"
#include "../alloc.hpp"
#include <lib/template-utils.hpp>
#include <lib/string-utils.hpp>
#include "filesystem.hpp"

String command;

// TODO: move this into string-utils
template <integral T>
T parse_num(StringView str) {
	T value = 0;
	const auto size = str.size();
	u8 base = 10;
	if (size == 0) return value;
	if (str[0] == '0') {
		if (size >= 3) {
			switch (str[1]) {
				case 'x':
				case 'X':
					base = 16;
					break;
				case 'b':
				case 'B':
					base = 2;
					break;
				case 'o':
				case 'O':
					base = 8;
					break;
				// uhh error
			}
			str = str.sub(2);
		} else {
			// uhh error
		}
	}

	return parse_int<T>(str, base);
}

void execute_command(const StringView& command) {
	const auto [name, args] = command.split_once(' ');
	if (name == "echo"_sv) {
		terminal("{}", args);
	} else if (name == "mem") {
		size_t total = 0, used = 0;
		for (auto chunk = alloc::get_chunks(); chunk; chunk = chunk->next) {
			if (chunk->used)
				used += chunk->size;
			total += chunk->size;
		}
		terminal("Memory info:\nUsed: {} ({}%)\nAvailable: {}"_sv, used, 100 * used / total, total);
	} else if (name == "alloc"_sv) {
		const auto value = parse_num<size_t>(args);
		auto ptr = malloc(value);
		terminal("Allocated {} bytes at {}"_sv, value, ptr);
	} else if (name == "view"_sv) {
		auto ptr = parse_num<uptr>(args);
		terminal("got ptr 0x{x}\n"_sv, ptr);
		for (u32 j = 0; j < 4; ++j) {
			terminal("{08x} | "_sv, ptr);
			for (u32 i = 0; i < 16; ++i) {
				const auto b = *reinterpret_cast<u8*>(ptr + i);
				terminal("{02x} "_sv, b);
			}
			ptr += 16;
			terminal_put_char('\n');
		}
	} else if (name == "ls"_sv) {
		const auto files = kernel::filesystem::get_files();
		for (const auto& file : files) {
			terminal("{} - {} bytes\n"_sv, file.name, file.data.size());
		}
	} else if (name == "cat"_sv) {
		const auto file = kernel::filesystem::get_file(args);
		if (file) {
			for (const auto c : file.value()->data)
				terminal_put_char(c);
		}
	} else if (name == "put"_sv) {
		const auto [name, text] = args.split_once(' ');
		const auto file_or = kernel::filesystem::get_file(name);
		const auto file = file_or ? file_or.value() : kernel::filesystem::add_file(name);
		for (const auto i : text) {
			file->data.push_back(i);
		}
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

