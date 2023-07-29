#include <kernel/device/pic.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/window/manager.hpp>
#include <stl/string.hpp>

using namespace kernel::ps2;

struct Modifiers {
	bool ctrl = false;
	bool shift = false;
	bool alt = false;
	bool caps = false;
} modifiers;

Key key_map[256];

char apply_shift(Key key) {
	switch (key.ch) {
		case '1': return '!';
		case '2': return '@';
		case '3': return '#';
		case '4': return '$';
		case '5': return '%';
		case '6': return '^';
		case '7': return '&';
		case '8': return '*';
		case '9': return '(';
		case '0': return ')';
		case ',': return '<';
		case '.': return '>';
		case '/': return '?';
		case ';': return ':';
		case '\'': return '"';
		case '[': return '{';
		case ']': return '}';
		case '-': return '_';
		case '=': return '+';
		default: return key.ch;
	}
}

u64 long_scan_code = 0;

void kernel::ps2::handle_keyboard() {
	const auto byte = inb(PS2_DATA_PORT);
	if (byte == 0xe0) {
		long_scan_code = byte;
	} else {
		const bool pressed = !(byte & 0x80);
		const auto code = byte & ~0x80;

		auto key = key_map[code];

		if (key.kind == KeyKind::Printable) {
			if (pressed) {
				if (is_ascii_alpha(key.ch)) {
					// the character is uppercase by default
					key.ch = modifiers.shift != modifiers.caps ? key.ch : to_ascii_lowercase(key.ch);
				} else if (modifiers.shift) {
					key.ch = apply_shift(key);
				}
			}
		} else if (key.kind == KeyKind::LeftCtrl || key.kind == KeyKind::RightCtrl) {
			modifiers.ctrl = pressed;
		} else if (key.kind == KeyKind::LeftShift || key.kind == KeyKind::RightShift) {
			modifiers.shift = pressed;
		} else if (key.kind == KeyKind::LeftAlt || key.kind == KeyKind::RightAlt) {
			modifiers.alt = pressed;
		} else if (key.kind == KeyKind::CapsLock) {
			if (pressed) {
				modifiers.caps = !modifiers.caps;
			}
		} else {
			kdbg("({:02x})", byte);
		}

		if (window::WindowManager::initialized())
			window::WindowManager::get().handle_keyboard(key, pressed);
	}

	pic::send_eoi(1);
}

void kernel::ps2::init_keyboard() {
	// enable PS/2 keyboard
	pic::set_irq_mask(1, true);

	// mfw no enumeration in c++
	usize i = 0x10;
	for (char c : "QWERTYUIOP") {
		if (c == 0) break;
		key_map[i++] = Key{ KeyKind::Printable, c };
	}
	i = 0x1e;
	for (char c : "ASDFGHJKL;'") {
		if (c == 0) break;
		key_map[i++] = Key{ KeyKind::Printable, c };
	}
	i = 0x2c;
	for (char c : "ZXCVBNM,./") {
		if (c == 0) break;
		key_map[i++] = Key{ KeyKind::Printable, c };
	}
	i = 0x02;
	for (char c : "1234567890-=") {
		if (c == 0) break;
		key_map[i++] = Key{ KeyKind::Printable, c };
	}
	key_map[0x39] = Key{ KeyKind::Printable, ' ' };

	key_map[0x01] = Key{ KeyKind::Escape };
	key_map[0x1c] = Key{ KeyKind::Enter, '\n' };
	key_map[0x0e] = Key{ KeyKind::Backspace, '\x08' };

	key_map[0x1d] = Key{ KeyKind::LeftCtrl };
	key_map[0x2a] = Key{ KeyKind::LeftShift };
	key_map[0x36] = Key{ KeyKind::RightShift };
	key_map[0x38] = Key{ KeyKind::LeftAlt };
	key_map[0x3a] = Key{ KeyKind::CapsLock };
}