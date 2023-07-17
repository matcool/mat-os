#include <stl/types.hpp>
#include <kernel/idt.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>

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

	IDTEntry(const void* address, u16 segment, u8 ring, GateType gate) : segment(segment) {
		const auto addr = reinterpret_cast<uptr>(address);
		offset1 = addr >> 0;
		offset2 = addr >> 16;
		offset3 = addr >> 32;

		const u8 present = address ? 1 << 7 : 0;
		type_attributes = present | (ring << 5) | static_cast<u8>(gate);
	}

	// https://github.com/limine-bootloader/limine/blob/v5.x-branch/PROTOCOL.md#x86_64
	// 64-bit code descriptor is on index 5, so 0b101
	// last 3 bits should be 0, since i want to use the GDT and be on ring 0
	IDTEntry(const void* address) : IDTEntry(address, 0b101'000, 0, GateType::Interrupt) {}

	IDTEntry() : IDTEntry(nullptr) {}
};

static IDTEntry idt_table[256];

static struct [[gnu::packed]] {
	u16 size;
	void* addr;
} idt_register;

static_assert(sizeof(idt_register) == 10);
static_assert(sizeof(IDTEntry) == 16);

#define PUSH_REGS "\
	push %%r10; \
	push %%r11; \
	push %%r12; \
	push %%r13; \
	push %%r14; \
	push %%r15; \
	push %%r8;  \
	push %%r9;  \
	push %%rax; \
	push %%rbp; \
	push %%rbx; \
	push %%rcx; \
	push %%rdi; \
	push %%rdx; \
	push %%rsi;"

#define POP_REGS "\
	pop %%rsi; \
	pop %%rdx; \
	pop %%rdi; \
	pop %%rcx; \
	pop %%rbx; \
	pop %%rbp; \
	pop %%rax; \
	pop %%r9;  \
	pop %%r8;  \
	pop %%r15; \
	pop %%r14; \
	pop %%r13; \
	pop %%r12; \
	pop %%r11; \
	pop %%r10;"

static void kernel_interrupt_handler(u64 which, u64 error_code) {
	kdbgln("Ouchie! got into interrupt {}, with error code {:x}", which, error_code);
	halt();
}

// couldnt figure out how to just directly call it,
// maybe its just not possible on 64 bit
static auto* kernel_interrupt_handler_ptr = &kernel_interrupt_handler;

template <u64 Number>
[[gnu::naked]] void raw_interrupt_handler() {
	asm(PUSH_REGS R"asm(
		movq %0, %%rdi
		xor %%rsi, %%rsi
		call *%1
	)asm" POP_REGS "iretq" : /* output */ : "i"(Number), "m"(kernel_interrupt_handler_ptr));
}

template <u64 Number>
[[gnu::naked]] void raw_interrupt_error_handler() {
	// This pushes 15 registers
	// so error code will be [esp + 15 * 8]
	// [esp + 120] so 120(%esp) in ugly syntax
	asm(PUSH_REGS R"asm(
		movq %0, %%rdi
		movq 120(%%esp), %%rsi
		call *%1
	)asm" POP_REGS "iretq" : /* output */ : "i"(Number), "m"(kernel_interrupt_handler_ptr));
}

void kernel::idt::init() {
	for (usize i = 0; i < 256; ++i) {
		// defaults to not present
		idt_table[i] = IDTEntry();
	}

	([] <u64... Values> {
		((idt_table[Values] = IDTEntry(reinterpret_cast<void*>(&raw_interrupt_handler<Values>))), ...);
	}).operator()<0, 1, 2, 3, 4, 5, 6, 7, 9, 15, 16>();

	([] <u64... Values> {
		((idt_table[Values] = IDTEntry(reinterpret_cast<void*>(&raw_interrupt_error_handler<Values>))), ...);
	}).operator()<8, 10, 11, 12, 13, 14>();

	idt_register.size = sizeof(idt_table) - 1;
	idt_register.addr = &idt_table[0];

	asm volatile("lidt %0; sti" : : "m"(idt_register));

	kdbgln("IDT initialized");
}