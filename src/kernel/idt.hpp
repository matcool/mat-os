#pragma once
#include "common.hpp"

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

IDTEntry* idt_get_table();

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

void idt_init();