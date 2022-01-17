#include "pit.hpp"
#include "idt.hpp"
#include "log.hpp"
#include "pic.hpp"

constexpr u16 PIT_CHANNEL0_DATA = 0x40;
constexpr u16 PIT_COMMAND_REG = 0x43;

static u32 current_delay = 0;

INTERRUPT
void pit_interrupt(InterruptFrame*) {
	if (current_delay)
		--current_delay;
	pic_eoi(0);
}

void sleep(u32 delay) {
	current_delay = delay;
	while (current_delay)
		asm volatile("nop");
}

void pit_set_value(u16 value) {
	outb(PIT_CHANNEL0_DATA, value & 0xFF);
	outb(PIT_CHANNEL0_DATA, value >> 8);
}

void pit_init() {
	idt_get_table()[0x20] = IDTEntry(isr_wrapper<&pit_interrupt>, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);

	constexpr u8 CHANNEL = 0b00; // select channel 0
	constexpr u8 ACCESS_MODE = 0b11; // low byte and high byte mode
	constexpr u8 OPERATING_MODE = 0b011; // mode 3
	constexpr u8 BINARY_MODE = 0; // 16-bit binary
	outb(PIT_COMMAND_REG, CHANNEL << 6 | ACCESS_MODE << 4 | OPERATING_MODE << 1 | BINARY_MODE);

	// the pit runs at 1,193,182 Hz
	// 1,193,182 / 596 = 2001 hz
	pit_set_value(596);

	pic_clear_mask(0);
}
