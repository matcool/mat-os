#pragma once

#include <kernel/idt.hpp>
#include <kernel/memory/paging.hpp>
#include <stl/types.hpp>
#include <stl/vector.hpp>

namespace kernel::tasks {

static constexpr u8 SYSCALL_INTERRUPT_N = 0x80;

void yield_thread();

struct Thread {
	interrupt::Registers state;
	void* stack = nullptr;
	PhysicalAddress page_table;

	paging::PageTableEntry* get_page_entries() {
		const auto entries_addr = PhysicalAddress(page_table.value() & ~u64(0b11111));
		return reinterpret_cast<paging::PageTableEntry*>(entries_addr.to_virtual().ptr());
	}
};

class Scheduler {
	Vector<Thread> m_threads;
	usize m_active_idx = 0;

public:
	static Scheduler& get();
	static bool initialized();

	void init();

	void handle_interrupt(interrupt::Registers* regs);
};

}