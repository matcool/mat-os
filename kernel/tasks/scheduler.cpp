#include <kernel/gdt.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/syscall.hpp>
#include <kernel/tasks/scheduler.hpp>

using namespace kernel::tasks;

Scheduler& Scheduler::get() {
	DisableInterruptsGuard guard;
	static Scheduler instance;
	return instance;
}

void kernel::tasks::yield_thread() {
	kernel::syscall::raw_syscall(kernel::syscall::Syscalls::ThreadYield);
}

void screen_thread() {
	kernel::framebuffer::loop();
}

void Scheduler::init() {
	m_procs.push(allocate_kernel_process((void*)&screen_thread));
	auto* mem = kernel::alloc::allocate_page();
	u8 code[] = { 0x90, 0xfa, 0x48, 0x31, 0xC0, 0xCD, 0x80, 0x90, 0xEB, 0xF7, 0x90, 0xFA };
	memcpy(mem, code, sizeof(code));
	m_procs.push(allocate_process(mem));

	switch_context_to(&m_procs[m_active_idx]);
}

void Scheduler::kill_current_thread() {
	// TODO: leaks memory
	m_procs.remove(m_active_idx);
	if (m_active_idx >= m_procs.size()) m_active_idx = 0;
	switch_context_to(&m_procs[m_active_idx]);
}

void Scheduler::handle_interrupt(interrupt::Registers* regs) {
	if (m_procs.empty()) {
		panic("No threads in the scheduler!");
	}
	auto& cur_thread = m_procs[m_active_idx];
	cur_thread.thread.state = *regs;

	const auto next_index = (m_active_idx + 1) % m_procs.size();
	auto& next_thread = m_procs[next_index];
	m_active_idx = next_index;

	switch_context_to(&next_thread);
}

void kernel::tasks::switch_context_to(Process* process) {
	cli();
	// make use of hhdm to make sure regs is always accessible
	auto regs_fixed = VirtualAddress(&process->thread.state).to_hhdm().ptr();
	auto cr3 = process->page_table;
	asm volatile(R"asm(
		movw %2, %%ds
		movw %2, %%es
		movw %2, %%fs
		movw %2, %%gs

		movq %1, %%cr3
		movq %0, %%rsp
	)asm" ASM_POP_REGS "iretq"
	             :
	             : "r"(regs_fixed), "r"(cr3), "m"(process->thread.state.ss));
}