#include <kernel/gdt.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/memory/paging.hpp>
#include <kernel/tasks/process.hpp>
#include <stl/vector.hpp>

using namespace kernel;
using namespace kernel::tasks;

paging::PageTableEntry* Process::get_page_entries() {
	const auto entries_addr = PhysicalAddress(this->page_table.value() & ~u64(0b11111));
	return reinterpret_cast<paging::PageTableEntry*>(entries_addr.to_virtual().ptr());
}

Process tasks::allocate_process(void* entry_point) {
	Process proc;

	proc.page_table = alloc::allocate_physical_page();

	auto* kernel_pt = kernel::paging::get_base_entries();
	auto* thread_pt = proc.get_page_entries();
	for (int i = 0; i < 512; ++i) {
		if (i >= 256) {
			// higher half of memory, copy over kernel entries
			// they should be ring 0 only already
			thread_pt[i] = kernel_pt[i];
		} else {
			// blank entries
			thread_pt[i] = kernel::paging::PageTableEntry(0);
		}
	}

	proc.thread.state.rsp = 0x8000'0000 - sizeof(void*);
	proc.thread.state.rip = 0x0420'0000;

	auto stack = kernel::alloc::allocate_physical_page();
	kernel::paging::map_page(kernel::VirtualAddress(proc.thread.state.rsp), stack, thread_pt);

	kernel::paging::map_page(
		kernel::VirtualAddress(proc.thread.state.rip),
		kernel::VirtualAddress(entry_point).to_physical(),
		thread_pt
	);

	proc.thread.state.cs = gdt::USER_CODE_SEGMENT | 3;
	proc.thread.state.ss = gdt::USER_DATA_SEGMENT | 3;
	proc.thread.state.rflags = 0b1000000000; // interrupt enable flag

	return proc;
}

Process tasks::allocate_kernel_process(void* entry_point) {
	Process proc;

	const auto stack_pages = 2;

	proc.page_table = VirtualAddress(kernel::paging::get_base_entries());

	proc.thread.stack = kernel::alloc::allocate_pages(stack_pages);
	proc.thread.state.rsp =
		reinterpret_cast<uptr>(proc.thread.stack) + kernel::PAGE_SIZE * stack_pages - sizeof(void*);
	*reinterpret_cast<uptr*>(proc.thread.state.rsp) = 0;

	proc.thread.state.rip = reinterpret_cast<uptr>(entry_point);
	proc.thread.state.cs = kernel::gdt::KERNEL_CODE_SEGMENT;
	proc.thread.state.ss = kernel::gdt::KERNEL_DATA_SEGMENT;
	proc.thread.state.rflags = 0b1000000000; // interrupt enable flag

	return proc;
}