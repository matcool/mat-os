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
	u32 edi;
	u32 esi;
	u32 ebp;
	u32 esp;
	u32 ebx;
	u32 edx;
	u32 ecx;
	u32 eax;
	u32 eip;
	u16 cs;
	u32 eflags;
};


namespace {
	using isr_wrapper_t = void(__cdecl*)(InterruptFrame*);
	using isr_wrapper_exception_t = void(__cdecl*)(InterruptFrame*, u32);

	template <auto>
	static constexpr bool __false = false;

	template <auto func>
	struct __isr_wrapper {
		static_assert(__false<func>, "Improper function passed to isr_wrapper");
	};

	template <isr_wrapper_t func>
	struct __isr_wrapper<func> {
		__attribute__((naked))
		static void wrapper() {
			// god i hate at&t syntax
			asm volatile(R"(
				pushal
				mov %%esp, %%eax
				push %%eax
				call %P0
				pop %%eax
				popal
				iret
			)" : : "i"(func));
		}
	};

	template <isr_wrapper_exception_t func>
	struct __isr_wrapper<func> {
		__attribute__((naked))
		static void wrapper() {
			// god i hate at&t syntax
			// TODO: actually support error codes, rn they just break everything
			asm volatile(R"(
				add $4, %%esp
				pushal
				mov %%esp, %%eax
				push $0
				push %%eax
				call %P0
				pop %%eax
				popal
				iret
			)" : : "i"(func));
		}
	};
}

template <auto func>
static constexpr auto isr_wrapper = &__isr_wrapper<func>::wrapper;

#define INTERRUPT __attribute__((cdecl))


void idt_init();