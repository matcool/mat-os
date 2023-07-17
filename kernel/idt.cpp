#include <stl/types.hpp>
#include <stl/array.hpp>
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

struct Registers {
	// in reverse order that they were pushed in
	u64 rsi;
	u64 rdx;
	u64 rdi;
	u64 rcx;
	u64 rbx;
	u64 rbp;
	u64 rax;
	u64 r9;
	u64 r8;
	u64 r15;
	u64 r14;
	u64 r13;
	u64 r12;
	u64 r11;
	u64 r10;
	// pushed by the cpu
	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
};

enum class InterruptId : u64 {
	DivideZero = 0,
	NMI = 2,
	Breakpoint = 3,
	InvalidOpcode = 6,
	DoubleFault = 8,
	GeneralProtectionFault = 13,
	PageFault = 14,
};

static mat::StringView get_interrupt_name(u64 idx) {
	const auto names = mat::make_array<mat::StringView>(
		"Divide by 0",
		"Reserved",
		"NMI Interrupt",
		"Breakpoint (INT3)",
		"Overflow (INTO)",
		"Bounds range exceeded (BOUND)",
		"Invalid opcode (UD2)",
		"Device not available (WAIT/FWAIT)",
		"Double fault",
		"Coprocessor segment overrun",
		"Invalid TSS",
		"Segment not present",
		"Stack-segment fault",
		"General protection fault",
		"Page fault",
		"Reserved",
		"x87 FPU error",
		"Alignment check",
		"Machine check",
		"SIMD Floating-Point Exception"
	);
	if (idx < names.size()) {
		return names[idx];
	} else {
		return "Unknown";
	}
}

// which - rdi
// error_code - rsi
// regs - rdx
static void kernel_interrupt_handler(u64 which, u64 error_code, Registers* regs) {
	kdbgln("[INT] {}, with error code {:x}", get_interrupt_name(which), error_code);
	const auto id = static_cast<InterruptId>(which);
	if (id == InterruptId::PageFault) {
		kdbgln("[page fault] {} on {} at {:#08x} by {}",
			error_code & 1 ? "Page-protection violation" : "Non-present page",
			error_code & 0b10 ? "write" : "read",
			get_cr2(),
			error_code & 0b100 ? "user" : "kernel"
		);
	}
	kdbgln("rip - {:#x}", regs->rip);
	kdbgln("rsp - {:#x}", regs->rsp);
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
		movq %%rsp, %%rdx
		call *%1
	)asm" POP_REGS "iretq" : /* output */ : "i"(Number), "m"(kernel_interrupt_handler_ptr));
}

// a place to store the error code before the registers are saved
u64 error_code_storage;

template <u64 Number>
[[gnu::naked]] void raw_interrupt_error_handler() {
	// pops off the error code first,
	// so that the stack looks the same to a non error handler
	asm("popq %2;\n\t" PUSH_REGS R"asm(
		movq %0, %%rdi
		movq %2, %%rsi
		movq %%rsp, %%rdx
		call *%1
	)asm" POP_REGS "iretq" : /* output */ : "i"(Number), "m"(kernel_interrupt_handler_ptr), "m"(error_code_storage));
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