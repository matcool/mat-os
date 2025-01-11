#pragma once

#include <stl/types.hpp>

namespace kernel::interrupt {

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
	u64 ss;
};

#define ASM_PUSH_REGS \
	"\
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

#define ASM_POP_REGS \
	"\
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

void init();

}