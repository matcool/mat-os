#include "idt.hpp"
#include "serial.hpp"
#include "log.hpp"
#include "terminal.hpp"

static struct {
	u16 size;
	u32 addr;
} PACKED idt_register;

static_assert(sizeof(idt_register) == 6, "Zoink");

static kernel::InterruptDescriptorTable::Entry idt_table[256];

template <size_t N>
INTERRUPT
void fancy_interrupt(kernel::InterruptFrame* frame) {
	serial("EIP: {x}\nESP: {x}\nhit interrupt {}\n", frame->eip, frame->esp, N);
}

template <size_t N>
constexpr void gen_interrupts_lol() {
	if (idt_table[N].attributes == 0)
		idt_table[N] = kernel::InterruptDescriptorTable::Entry(kernel::isr_wrapper<&fancy_interrupt<N>>, kernel::IDT_GATE | kernel::IDT_GATE_INTERRUPT, 8);
	if constexpr (N != 0)
		gen_interrupts_lol<N - 1>();
}

template <size_t N>
INTERRUPT
void fancy_exception(kernel::InterruptFrame* frame, u32 error_code) {
	serial("EIP: {x}\nESP: {x}\nError code: {}\nhit interrupt {}\n", frame->eip, frame->esp, error_code, N);
}

template <size_t N>
void gen_exception() {
	if (idt_table[N].attributes == 0)
		idt_table[N] = kernel::InterruptDescriptorTable::Entry(kernel::isr_wrapper<&fancy_exception<N>>, kernel::IDT_GATE | kernel::IDT_GATE_INTERRUPT, 0x08);
}

void kernel::InterruptDescriptorTable::init() {
	gen_exception<8>();
	gen_exception<10>();
	gen_exception<11>();
	gen_exception<12>();
	gen_exception<13>();
	gen_exception<14>();
	gen_exception<17>();
	gen_exception<21>();
	gen_exception<30>();

	gen_interrupts_lol<31>();

	idt_register.size = sizeof(idt_table) - 1;
	idt_register.addr = reinterpret_cast<uptr>(idt_table);

	log("IDT initialized");

	asm volatile("lidt %0; sti" : : "m"(idt_register));

#if 0

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
#endif
}

void kernel::InterruptDescriptorTable::set_entry(size_t index, void* function, u8 attributes, u16 selector) {
	idt_table[index] = kernel::InterruptDescriptorTable::Entry(function, attributes, selector);
}
