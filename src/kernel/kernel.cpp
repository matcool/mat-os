#include "kernel.h"
#include "../common.hpp"

// Check if the compiler thinks you are targeting the wrong operating system.
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
// This tutorial will only work for the 32-bit ix86 targets.
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif
 
// Hardware text mode color constants.
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};
 
uint8_t vga_entry_color(vga_color fg, vga_color bg)  {
	return fg | bg << 4;
}
 
uint16_t vga_entry(unsigned char uc, uint8_t color)  {
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void* memcpy(void* dst, const void* src, size_t len) {
	// TODO: optimize this
	auto d = (uint8_t*)dst;
	auto s = (const uint8_t*)src;
	while (len--) {
		*d++ = *s++;
	}
	return dst;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize() {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_scroll_down() {
	--terminal_row;
	for (size_t y = 1; y < VGA_HEIGHT; y++) {
		memcpy(&terminal_buffer[(y - 1) * VGA_WIDTH], &terminal_buffer[y * VGA_WIDTH], VGA_WIDTH * sizeof(*terminal_buffer));
	}
}
 
void terminal_set_color(uint8_t color) {
	terminal_color = color;
}
 
void terminal_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_put_char(char c) {
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
		return;
	}
	terminal_put_entry_at(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	}
}
 
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_put_char(data[i]);
}
 
void terminal_write_string(const char* data) {
	terminal_write(data, strlen(data));
}

static uint8_t gdt_raw[8][3];

struct GDTEntry {
	size_t base;
	size_t limit;
	uint8_t type;
};

void gdt_initialize() {
	GDTEntry table[3];

	table[0] = {.base=0, .limit=0, .type=0};               // Selector 0x00 cannot be used
	table[1] = {.base=0, .limit=0xffffffff, .type=0x9A};   // Selector 0x08 will be our code
	table[2] = {.base=0, .limit=0xffffffff, .type=0x92};   // Selector 0x10 will be our data

	// TODO: task state segment entry

	for (size_t i = 0; i < 3; ++i) {
		auto& entry = table[i];
		auto raw = gdt_raw[i];
		if (entry.limit > 65536) {
			entry.limit >>= 12;
			raw[6] = 0xC0;
		} else {
			raw[6] = 0x40;
		}

		raw[0] = entry.limit & 0xFF;
		raw[1] = (entry.limit >> 8) & 0xFF;
		raw[6] |= (entry.limit >> 16) & 0xF;

		raw[2] = entry.base & 0xFF;
		raw[3] = (entry.base >> 8) & 0xFF;
		raw[4] = (entry.base >> 16) & 0xFF;
		raw[7] = (entry.base >> 24) & 0xFF;
	
		raw[5] = entry.type;
	}

	set_gdt(gdt_raw, sizeof(gdt_raw));
}

#define PACKED __attribute__((packed))

struct IDTEntry {
	uint16_t isr_low;   // The lower 16 bits of the ISR's address
	uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t reserved;   // Set to zero
	uint8_t attributes; // Type and attributes; see the IDT page
	uint16_t isr_high;  // The higher 16 bits of the ISR's address
} PACKED;

constexpr const auto IDT_ENTRIES = 256;
static IDTEntry idt_table[IDT_ENTRIES];

static struct {
	uint16_t limit;
	uint32_t base;
} PACKED idt_reg; // the r stands for register

__attribute__((noreturn))
extern "C" void exception_handler() {
	asm("cli\n"
	    "hlt":);
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
	auto& descriptor = idt_table[vector];

	descriptor.isr_low    = reinterpret_cast<uint32_t>(isr) & 0xFFFF;
	descriptor.kernel_cs  = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
	descriptor.attributes = flags;
	descriptor.isr_high   = reinterpret_cast<uint32_t>(isr) >> 16;
	descriptor.reserved   = 0;
}

extern void* isr_stub_table[];

void idt_initialize() {
	idt_reg.base = reinterpret_cast<uintptr_t>(&idt_table[0]);
	idt_reg.limit = uint16_t(sizeof(IDTEntry) * IDT_ENTRIES - 1);

	for (u8 vector = 0; vector < 32; ++vector) {
		idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
	}

	asm volatile("lidt %0\n"
	             "sti" : : "m"(idt_reg));
	// sti enabled interrupts by setting the interrupt flag
}

extern "C" void kernel_main() {
	// Initialize terminal interface
	terminal_initialize();

	gdt_initialize();

	idt_initialize();

	terminal_set_color(2);
 
	terminal_write_string("Hello, kernel World!\n");
	terminal_write_string("I am mat\n");
	terminal_write_string("I am mat 2");
	terminal_write_string("its me mat once again");

	terminal_scroll_down();

}
