#include "kernel.hpp"
#include "terminal.hpp"
#include "stdio.hpp"

// Check if the compiler thinks you are targeting the wrong operating system.
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
// This tutorial will only work for the 32-bit ix86 targets.
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

static uint8_t gdt_raw[8][3];

struct GDTEntry {
	size_t base;
	size_t limit;
	uint8_t type;
};

extern "C" void set_gdt(void*, size_t);

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
 
	printf("Hello, kernel World!\n");
	printf("I am mat\n");
	printf("I am mat 2");
	printf("its me mat once again");

	terminal_scroll_down();

}
