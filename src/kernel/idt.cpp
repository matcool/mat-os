#include "idt.hpp"
#include "serial.hpp"

static struct {
	u16 size;
	u32 addr;
} PACKED idt_register;

static_assert(sizeof(idt_register) == 6, "Zoink");

struct IDTEntry {
	u16 addr_low;  // lower 2 bytes of the address
	u16 selector;  // gdt selector
	u8 unused;     // always 0
	u8 attributes; //
	u16 addr_high; // higher 2 bytes of address

	IDTEntry() : unused(0), attributes(0) {};

	template <class T>
	IDTEntry(T func, u8 attributes, u16 selector) : selector(selector), unused(0), attributes(attributes) {
		const auto addr = reinterpret_cast<uptr>(func);
		addr_low = addr & 0xFFFF;
		addr_high = addr >> 16;
	}
} PACKED;

static_assert(sizeof(IDTEntry) == 8, "Zoink");

static IDTEntry idt_table[256];

// Interrupt gates:
// 1 00 0 1110
// │ │  │ │
// │ │  │ └ Gate type: set as 32 bit interrupt
// │ │  └ Magical zero bit
// │ └ DPL: defines the priviledge level
// └ Present bit, must be set for valid gates

constexpr auto IDT_GATE       = 0b10000000;
constexpr auto IDT_GATE_INTERRUPT = 0b1110;
constexpr auto IDT_GATE_TRAP      = 0b1111;
constexpr auto IDT_GATE_TASK      = 0b0101;

struct InterruptFrame {

};

__attribute__((interrupt))
void interrupt_handler(InterruptFrame* frame) {
	serial_put_string("interrupt handler hit\n");
}

__attribute__((interrupt))
void exception_handler(InterruptFrame* frame, u32 error_code) {
	serial_put_string("exception handler hit\n");
}

void idt_init() {
	for (size_t i = 0; i < 32; ++i) {
		idt_table[i] = IDTEntry(&interrupt_handler, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);
	}
	constexpr size_t errors[10] = { 8, 10, 11, 12, 13, 14, 17, 21, 29, 30 };
	for (const auto i : errors) {
		idt_table[i] = IDTEntry(&exception_handler, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);
	}

	idt_register.size = sizeof(idt_table) - 1;
	// TODO: change this when paging is implemented
	idt_register.addr = reinterpret_cast<uptr>(idt_table);

	serial_put_string("going to load idt\n");

	asm volatile("lidt %0" : : "m"(idt_register));

	struct {
		u16 size;
		uptr addr;
	} PACKED data;

	asm volatile("sidt %0" : : "m"(data));

	if (data.size != sizeof(idt_table) - 1) {
		serial_put_string("wadafak happened\n");
	}
	if (data.addr != reinterpret_cast<uptr>(idt_table)) {
		serial_put_string("not equal to this\n");
	}
	if (data.addr != reinterpret_cast<uptr>(&idt_table)) {
		serial_put_string("not equal to this either\n");
	}

	serial_put_string("IDT: ");

	serial_put_number(data.size);
	serial_put_char(' ');
	serial_put_hex(data.addr);
	serial_put_string("\n");
}