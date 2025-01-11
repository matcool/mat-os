#include <kernel/gdt.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/syscall.hpp>
#include <kernel/tasks/scheduler.hpp>

using namespace kernel::tasks;

bool initialized = false;

bool Scheduler::initialized() {
	return ::initialized;
}

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

Thread create_kernel_thread(usize stack_pages, void (*function)()) {
	Thread thread;
	thread.page_table = kernel::PhysicalAddress(get_cr3());
	thread.stack = kernel::alloc::allocate_pages(stack_pages);
	thread.state.rsp =
		reinterpret_cast<uptr>(thread.stack) + kernel::PAGE_SIZE * stack_pages - sizeof(uptr);
	*reinterpret_cast<uptr*>(thread.state.rsp) = 0;
	thread.state.rip = reinterpret_cast<uptr>(function);
	thread.state.cs = kernel::gdt::KERNEL_CODE_SEGMENT;
	thread.state.ss = kernel::gdt::KERNEL_DATA_SEGMENT;
	thread.state.rflags = 0b1000000000; // interrupt enable flag
	kdbgln("thread rsp={:#x}", thread.state.rsp);
	return thread;
}

Thread create_user_thread(usize stack_pages, void* function) {
	Thread thread;
	thread.page_table = kernel::alloc::allocate_physical_page();
	constexpr auto n_entries = 512;
	auto* kernel_pt = kernel::paging::get_base_entries();
	auto* thread_pt = thread.get_page_entries();
	for (int i = 0; i < n_entries; ++i) {
		if (i >= 256) {
			// higher half of memory, copy over kernel entries
			// they should be ring 0 only already
			thread_pt[i] = kernel_pt[i];
		} else {
			// blank entries
			thread_pt[i] = kernel::paging::PageTableEntry(0);
		}
	}

	thread.state.rsp = 0x8000'0000 - 8;
	auto stack = kernel::alloc::allocate_physical_page();
	kernel::paging::map_page(kernel::VirtualAddress(thread.state.rsp), stack, thread_pt);
	thread.state.rip = 0x0420'0000;
	kernel::paging::map_page(
		kernel::VirtualAddress(thread.state.rip), kernel::VirtualAddress(function).to_physical(), thread_pt
	);
	thread.state.cs = kernel::gdt::USER_CODE_SEGMENT | 3;
	thread.state.ss = kernel::gdt::USER_DATA_SEGMENT | 3;
	thread.state.rflags = 0b1000000000; // interrupt enable flag
	kdbgln("thread rsp={:#x}", thread.state.rsp);
	return thread;
}

void Scheduler::init() {
	m_threads.push(create_kernel_thread(3, &screen_thread));
	auto* mem = kernel::alloc::allocate_page();
	u8 code[] = { 0x90, 0x90, 0x48, 0x31, 0xC0, 0xCD, 0x80, 0x90, 0xEB, 0xF7, 0x90, 0xFA };
	memcpy(mem, code, sizeof(code));
	m_threads.push(create_user_thread(2, mem));
	::initialized = true;

	switch_context_to(&m_threads[m_active_idx]);
}

void Scheduler::handle_interrupt(interrupt::Registers* regs) {
	if (m_threads.empty()) {
		panic("No threads in the scheduler!");
	}
	auto& cur_thread = m_threads[m_active_idx];
	cur_thread.state = *regs;

	const auto next_index = (m_active_idx + 1) % m_threads.size();
	auto& next_thread = m_threads[next_index];
	m_active_idx = next_index;

	switch_context_to(&next_thread);
}

void kernel::tasks::switch_context_to(Thread* thread) {
	cli();
	// make use of hhdm to make sure regs is always accessible
	auto regs_fixed = VirtualAddress(&thread->state).to_hhdm().ptr();
	auto cr3 = thread->page_table;
	asm volatile(R"asm(
		movw %2, %%ds
		movw %2, %%es
		movw %2, %%fs
		movw %2, %%gs

		movq %1, %%cr3
		movq %0, %%rsp
	)asm" ASM_POP_REGS "iretq"
	             :
	             : "r"(regs_fixed), "r"(cr3), "m"(thread->state.ss));
}