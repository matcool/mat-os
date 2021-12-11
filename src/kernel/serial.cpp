#include "serial.hpp"
#include "common.hpp"

void serial_init() {
	outb(COM1 + 1, 0x00);    // Disable all interrupts
	outb(COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outb(COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(COM1 + 1, 0x00);    //                  (hi byte)
	outb(COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
	outb(COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_put_char(char c) {
	if (c == '\n') serial_put_char('\r');
	while ((inb(COM1 + 5) & 0x20) == 0);
	outb(COM1, c);
}

void serial_put_string(const char* str) {
	size_t i = 0;
	char c;
	while ((c = str[i++])) {
		serial_put_char(c);
	}
}

void serial_put_number(i32 n) {
	char buffer[11] = {};
	size_t i = 11;
	const bool neg = n < 0;
	do {
		buffer[--i] = '0' + (n % 10);
		n /= 10;
	} while (n);
	if (neg) buffer[--i] = '-';
	while (i < 11)
		serial_put_char(buffer[i++]);
}

void serial_put_hex(u32 n) {
	char buffer[10] = {};
	size_t i = 10;
	do {
		u8 c = n & 0xF;
		c += c > 9 ? 'A' - 10 : '0';
		buffer[--i] = c;
		n >>= 4;
	} while (n);
	buffer[--i] = 'x';
	buffer[--i] = '0';
	while (i < 10)
		serial_put_char(buffer[i++]);
}