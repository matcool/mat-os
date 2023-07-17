#include <kernel/serial.hpp>
#include <kernel/intrinsics.hpp>

static constexpr u16 COM1 = 0x3F8;

namespace kernel::serial {
	void init() {
		outb(COM1 + 1, 0x00); // Disable all interrupts
		outb(COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
		outb(COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
		outb(COM1 + 1, 0x00); //                  (hi byte)
		outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
		outb(COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
		outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
	}

	void put_byte(u8 value) {
		outb(COM1, value);
	}

	void put_char(char value) {
		put_byte(value);
	}

	void put(mat::StringView str) {
		for (char c : str) {
			put_char(c);
		}
	}
}