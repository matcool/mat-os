#include <stl/string.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/device/pic.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/screen/terminal.hpp>

enum class KeyKind {
	Other,
	Printable,

	Escape,
	Enter,
	Backspace,
	CapsLock,
	Left,
	Right,
	Up,
	Down,
	
	LeftCtrl,
	RightCtrl,
	LeftShift,
	RightShift,
	LeftAlt,
	RightAlt
};

struct Modifiers {
	bool ctrl = false;
	bool shift = false;
	bool alt = false;
	bool caps = false;
} modifiers;

struct Key {
	KeyKind kind = KeyKind::Other;
	char ch = 0;
};

Key key_map[256];

u64 long_scan_code = 0;

void kernel::ps2::handle_keyboard() {
	const auto byte = inb(PS2_DATA_PORT);
	if (byte == 0xe0) {
		long_scan_code = byte;
	} else {
		const bool pressed = !(byte & 0x80);
		const auto code = byte & ~0x80;

		const auto key = key_map[code];
		
		if (key.kind == KeyKind::Printable) {
			if (pressed) {
				const char ch = modifiers.shift ^ modifiers.caps ? key.ch : mat::to_ascii_lowercase(key.ch);
				// kdbg("{}", ch);
				kernel::terminal::type_character(ch);
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
	}
	
	pic::send_eoi(1);
}

void kernel::ps2::init_keyboard() {
	// enable PS/2 keyboard
	pic::set_irq_mask(1, true);

	// mfw no enumeration in c++
	usize i = 0x10;
	for (char c : "QWERTYUIOP") {
		key_map[i++] = Key { KeyKind::Printable, c };
	}
	i = 0x1e;
	for (char c : "ASDFGHJKL") {
		key_map[i++] = Key { KeyKind::Printable, c };
	}
	i = 0x2c;
	for (char c : "ZXCVBNM") {
		key_map[i++] = Key { KeyKind::Printable, c };
	}
	i = 0x02;
	for (char c : "1234567890") {
		key_map[i++] = Key { KeyKind::Printable, c };
	}
	key_map[0x39] = Key { KeyKind::Printable, ' ' };
	
	key_map[0x01] = Key { KeyKind::Escape };
	key_map[0x1d] = Key { KeyKind::LeftCtrl };
	key_map[0x2a] = Key { KeyKind::LeftShift };
	key_map[0x36] = Key { KeyKind::RightShift };
	key_map[0x38] = Key { KeyKind::LeftAlt };
	key_map[0x3a] = Key { KeyKind::CapsLock };
}