#include <kernel/syscall.hpp>
#include <kernel/tasks/scheduler.hpp>

using namespace kernel::syscall;

void kernel::syscall::handle_syscall(interrupt::Registers* regs) {
	const auto number = static_cast<Syscalls>(regs->rax);
	if (number == Syscalls::ThreadYield) {
		if (kernel::tasks::Scheduler::initialized())
			kernel::tasks::Scheduler::get().handle_interrupt(regs);
	}
}