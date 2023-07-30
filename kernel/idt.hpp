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
};

void init();

}