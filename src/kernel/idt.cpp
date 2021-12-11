#include "idt.hpp"
#include "serial.hpp"

static struct {
	u16 size;
	u32 addr;
} PACKED idt_register;

static_assert(sizeof(idt_register) == 6, "Zoink");

static IDTEntry idt_table[256];

IDTEntry* idt_get_table() { return idt_table; }

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
		if (!idt_table[i].attributes)
			idt_table[i] = IDTEntry(&interrupt_handler, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);
	}
	constexpr size_t errors[10] = { 8, 10, 11, 12, 13, 14, 17, 21, 29, 30 };
	for (const auto i : errors) {
		if (!idt_table[i].attributes)
			idt_table[i] = IDTEntry(&exception_handler, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);
	}

	serial_put_string("the number: ");
	serial_put_number(idt_table[0x21].attributes);
	serial_put_char('\n');

	idt_register.size = sizeof(idt_table) - 1;
	// TODO: change this when paging is implemented
	idt_register.addr = reinterpret_cast<uptr>(idt_table);

	asm volatile("lidt %0; sti" : : "m"(idt_register));

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