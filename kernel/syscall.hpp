#pragma once

#include <kernel/idt.hpp>
#include <stl/types.hpp>
#include <stl/utils.hpp>

namespace kernel::syscall {

static constexpr u8 SYSCALL_INTERRUPT_N = 0x80;

template <class... Args>
uptr raw_syscall(uptr number, Args... args) {
	static_assert(sizeof...(Args) <= 2);
	uptr result;
	// clang-format off
	overloaded {
		[&]() {
			asm volatile(
				"mov %1, %%rax;"
				"int %2;"
				"mov %%rax, %0"
				: "=r"(result)
				: "i"(number), "i"(SYSCALL_INTERRUPT_N)
			);
		},
		[&](auto arg1) {
			asm volatile(
				"mov %1, %%rax;"
				"mov %3, %%r9;"
				"int %2;"
				"mov %%rax, %0"
				: "=r"(result)
				: "i"(number), "i"(SYSCALL_INTERRUPT_N), "m"(arg1)
			);
		},
		[&](auto arg1, auto arg2) {
			asm volatile(
				"mov %1, %%rax;"
				"mov %3, %%r9;"
				"mov %4, %%r10;"
				"int %2;"
				"mov %%rax, %0"
				: "=r"(result)
				: "i"(number), "i"(SYSCALL_INTERRUPT_N), "m"(arg1), "m"(arg2)
			);
		},
	}(args...);
	// clang-format on
	return result;
}

void handle_syscall(interrupt::Registers* regs);

enum Syscalls : u64 {
	ThreadYield = 0
};

}