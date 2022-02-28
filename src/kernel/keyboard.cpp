#include "keyboard.hpp"
#include "idt.hpp"
#include "serial.hpp"
#include "terminal.hpp"
#include "pic.hpp"
#include "log.hpp"
#include "ps2.hpp"
#include "screen.hpp"
#include <lib/string.hpp>

char scan_code_map[256] = {0};

bool shift_held = false;
bool caps_lock = false;

INTERRUPT
void keyboard_interrupt(kernel::InterruptFrame*) {
	auto scan_code = inb(PS2_DATA_PORT);
	// serial("scan code: {x}\n", scan_code);
	if (scan_code == 0xFA) {

	} else {
		const bool release = scan_code & 0x80;
		scan_code &= ~0x80;
		if (scan_code == 0x2a) {
			shift_held = !release;
		} else if (!release) {
			auto& dispatcher = KeyboardDispatcher::get();
			if (scan_code == 0xe) {
				dispatcher.dispatch(8); // Backspace
			} else if (scan_code == 0x1c) {
				dispatcher.dispatch('\n');
			} else {
				char c = scan_code_map[scan_code];
				if (c) {
					if (!shift_held && c >= 'A' && c <= 'Z')
						c += 32;
					else if (shift_held && c >= '0' && c <= '9') {
						static const char shift_numbers[10] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};
						c = shift_numbers[c - '0'];
					}
					dispatcher.dispatch(c);
				}
			}
		}
	}
	pic_eoi(1);
}

void keyboard_init() {
	u8 i = 0x10;
	for (const auto c : "QWERTYUIOP"_sv)
		scan_code_map[i++] = c;
	i = 0x1e;
	for (const auto c : "ASDFGHJKL"_sv)
		scan_code_map[i++] = c;
	i = 0x2c;
	for (const auto c : "ZXCVBNM,.;"_sv)
		scan_code_map[i++] = c;
	scan_code_map[0x39] = ' ';
	scan_code_map[0xB] = '0';
	for (i = 1; i < 10; ++i)
		scan_code_map[i + 1] = '0' + i;

	kernel::InterruptDescriptorTable::set_entry(0x21, kernel::isr_wrapper<&keyboard_interrupt>);
	log("Keyboard initialized");
}

static KeyboardDispatcher s_dispatcher;
KeyboardDispatcher& KeyboardDispatcher::get() {
	return s_dispatcher;
}

void KeyboardDispatcher::add_listener(Function<void(u8)> callback) {
	m_listeners.push_back(callback);
}

void KeyboardDispatcher::dispatch(u8 key) {
	for (auto& cb : m_listeners) {
		cb(key);
	}
}
