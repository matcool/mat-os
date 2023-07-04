#include "idt.hpp"
#include <stl/types.hpp>
#include "serial.hpp"

struct IDTEntry {
	u16 offset1;
	u16 segment;
	u8 ist = 0;
	u8 type_attributes;
	u16 offset2;
	u32 offset3;
	u32 reserved = 0;

	enum class GateType {
		Interrupt = 0xE,
		Trap = 0xF
	};

	IDTEntry(void* address, u16 segment, u8 ring, GateType gate) : segment(segment) {
		const auto addr = reinterpret_cast<uptr>(address);
		offset1 = addr >> 0;
		offset2 = addr >> 16;
		offset3 = addr >> 32;

		// first one is present bit
		type_attributes = 0b10000000 | (ring << 5) | static_cast<u8>(gate);
	}

	IDTEntry() : IDTEntry(nullptr, 0, 0, GateType::Interrupt) {}
};

static IDTEntry idt_table[256];

static struct [[gnu::packed]] {
	u16 size;
	void* addr;
} idt_register;

static_assert(sizeof(idt_register) == 10);

void kernel::idt::init() {
	idt_register.size = sizeof(idt_table) - 1;
	idt_register.addr = idt_table;
	asm volatile("lidt %0" : : "m"(idt_register));

	serial::fmtln("IDT initialized");
}