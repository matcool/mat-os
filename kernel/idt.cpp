#include "idt.hpp"
#include <stl/types.hpp>
#include "serial.hpp"

void cool_function() {
	kernel::serial::fmtln("whats up!");
}

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
		type_attributes = 0b1000'0000 | (ring << 5) | static_cast<u8>(gate);
	}

	// https://github.com/limine-bootloader/limine/blob/v5.x-branch/PROTOCOL.md#x86_64
	// 64-bit code descriptor is on index 5, so 0b101
	// last 3 bits should be 0, since i want to use the GDT and be on ring 0
	IDTEntry() : IDTEntry(reinterpret_cast<void*>(&cool_function), 0b101'000, 0, GateType::Interrupt) {}
};

static IDTEntry idt_table[256];

static struct [[gnu::packed]] {
	u16 size;
	void* addr;
} idt_register;

static_assert(sizeof(idt_register) == 10);
static_assert(sizeof(IDTEntry) == 16);

void kernel::idt::init() {
	for (int i = 0; i < 256; ++i) {
		// static init constructors dont run yet!
		idt_table[i] = IDTEntry();
	}

	idt_register.size = sizeof(idt_table) - 1;
	idt_register.addr = &idt_table[0];

	serial::fmtln("Real has size {}, addr is {:x}", idt_register.size, reinterpret_cast<uptr>(idt_register.addr));

	asm volatile("lidt %0; sti" : : "m"(idt_register));

	serial::fmtln("IDT initialized");

	// sanity checking

	decltype(idt_register) copy;

	asm volatile("sidt %0" : : "m"(copy));

	serial::fmtln("Copy has size {}, addr is {:x}", copy.size, reinterpret_cast<uptr>(copy.addr));
}