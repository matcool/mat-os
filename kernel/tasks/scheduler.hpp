#pragma once

#include <kernel/idt.hpp>
#include <kernel/memory/paging.hpp>
#include <kernel/tasks/process.hpp>
#include <stl/types.hpp>
#include <stl/vector.hpp>

namespace kernel::tasks {

class Scheduler {
	Vector<Process> m_procs;
	usize m_active_idx = 0;

public:
	static Scheduler& get();

	void init();

	void handle_interrupt(interrupt::Registers* regs);

	void kill_current_thread();

	void add_process(Process proc);
};

void yield_thread();
void switch_context_to(Process* proc);

}