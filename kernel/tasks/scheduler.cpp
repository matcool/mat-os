#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/screen/framebuffer.hpp>
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
	asm volatile("int %0;" : : "i"(SYSCALL_INTERRUPT_N));
}

void thread1_func() {
	while (true) {
		// kdbgln("hello from thread 1");
		yield_thread();
	}
	halt(false);
}

void thread2_func() {
	while (true) {
		// kdbgln("hello from thread 2");
		yield_thread();
	}
	halt(false);
}

void thread3_func() {
	kernel::framebuffer::loop();
}

Thread create_thread(usize stack_pages, void (*function)()) {
	Thread thread;
	thread.stack = kernel::alloc::allocate_pages(stack_pages);
	thread.state.rsp =
		reinterpret_cast<uptr>(thread.stack) + kernel::PAGE_SIZE * stack_pages - sizeof(uptr);
	*reinterpret_cast<uptr*>(thread.state.rsp) = 0;
	thread.state.rip = reinterpret_cast<uptr>(function);
	thread.state.cs = 0;
	kdbgln("thread rsp={:#x}", thread.state.rsp);
	return thread;
}

void Scheduler::init() {
	m_threads.push(create_thread(1, &thread1_func));
	m_threads.push(create_thread(1, &thread2_func));
	m_threads.push(create_thread(3, &thread3_func));
	::initialized = true;
	yield_thread(); // jump into the scheduler interrupt
}

kernel::interrupt::Registers kernel_init_state;

void Scheduler::handle_interrupt(interrupt::Registers* regs) {
	if (kernel_init_state.rip == 0) {
		kernel_init_state = *regs;
	}

	if (m_threads.empty()) return;

	if (!m_threads[m_active_idx].first_time) {
		m_threads[m_active_idx].state = *regs;
	}
	const auto next_index = (m_active_idx + 1) % m_threads.size();
	auto& next_thread = m_threads[next_index];
	// silly
	if (next_thread.first_time) {
		next_thread.state.cs = kernel_init_state.cs;
		next_thread.state.rflags = 0b1000000000; // interrupt enable flag
		next_thread.first_time = false;
	}
	*regs = next_thread.state;
	m_active_idx = next_index;
}