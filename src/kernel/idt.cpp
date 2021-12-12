#include "idt.hpp"
#include "serial.hpp"

static struct {
	u16 size;
	u32 addr;
} PACKED idt_register;

static_assert(sizeof(idt_register) == 6, "Zoink");

static IDTEntry idt_table[256];

IDTEntry* idt_get_table() { return idt_table; }

INTERRUPT
void exception_handler(InterruptFrame*, u32) {
	serial_put_string("exception handler hit\n");
}

template <size_t N>
INTERRUPT
void fancy_interrupt(InterruptFrame* frame) {
	serial_put_string("EIP: "); serial_put_hex(frame->eip); serial_put_char('\n');
	serial_put_string("ESP: "); serial_put_hex(frame->esp); serial_put_char('\n');
	serial_put_string("hit interrupt "); serial_put_number(N); serial_put_char('\n');
}

template <size_t N>
constexpr void gen_interrupts_lol() {
	if (idt_table[N].attributes == 0)
		idt_table[N] = IDTEntry(isr_wrapper<&fancy_interrupt<N>>, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);
	if constexpr (N != 0)
		gen_interrupts_lol<N - 1>();
}

template <size_t N>
INTERRUPT
void fancy_exception(InterruptFrame* frame, u32 error_code) {
	serial_put_string("EIP: "); serial_put_hex(frame->eip); serial_put_char('\n');
	serial_put_string("ESP: "); serial_put_hex(frame->esp); serial_put_char('\n');
	serial_put_string("hit interrupt "); serial_put_number(N); serial_put_char('\n');
	serial_put_string("error code "); serial_put_number(error_code); serial_put_char('\n');
}

template <size_t N>
void gen_exception() {
	if (idt_table[N].attributes == 0)
		idt_table[N] = IDTEntry(isr_wrapper<&fancy_exception<N>>, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);
}

void idt_init() {
	// constexpr size_t errors[10] = { 8, 10, 11, 12, 13, 14, 17, 21, 29, 30 };
	// for (const auto i : errors) {
	// 	serial_put_number(i); serial_put_char(' '); serial_put_number(idt_table[i].attributes); serial_put_char(' '); serial_put_hex(idt_table[i].addr_low); serial_put_char('\n');
	// 	if (idt_table[i].attributes == 0)
	// 		idt_table[i] = IDTEntry(isr_wrapper<&exception_handler>, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);
	// }
	gen_exception<8>();
	gen_exception<10>();
	gen_exception<11>();
	gen_exception<12>();
	gen_exception<13>();
	gen_exception<14>();
	gen_exception<17>();
	gen_exception<21>();
	gen_exception<30>();
	serial_put_string("now going to the interrupt handlers\n");
	gen_interrupts_lol<31>();

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