#include <kernel/log.hpp>
#include <kernel/syscall.hpp>
#include <kernel/tasks/scheduler.hpp>

using namespace kernel::syscall;

void kernel::syscall::handle_syscall(interrupt::Registers* regs) {
	const auto number = static_cast<Syscalls>(regs->rax);
	if (number == Syscalls::ThreadYield) {
		kernel::tasks::Scheduler::get().handle_interrupt(regs);
	} else {
		panic("Invalid syscall triggered {:#x}", regs->rax);
	}
}