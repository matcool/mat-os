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

void test_thread() {
	int i = 0;
	while (true) {
		kdbgln("Hello! {}", i);
		++i;
		kernel::tasks::yield_thread();
	}
}

Thread create_thread(usize stack_pages, void (*function)()) {
	Thread thread;
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

void Scheduler::init() {
	m_threads.push(create_thread(3, &screen_thread));
	m_threads.push(create_thread(1, &test_thread));
	::initialized = true;
	// give control to scheduler, since this is the easiest way to switch to the first thread
	// TODO: write some switch_context function or whatever
	yield_thread();
}

// related to the comment above
bool very_first_time = true;

void Scheduler::handle_interrupt(interrupt::Registers* regs) {
	if (m_threads.empty()) {
		panic("No threads in the scheduler!");
	}
	auto& cur_thread = m_threads[m_active_idx];
	if (!very_first_time) {
		cur_thread.state = *regs;
	} else {
		very_first_time = false;
	}

	const auto next_index = (m_active_idx + 1) % m_threads.size();
	auto& next_thread = m_threads[next_index];
	*regs = next_thread.state;
	m_active_idx = next_index;
}