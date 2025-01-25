#pragma once

#include <kernel/idt.hpp>
#include <kernel/memory/paging.hpp>
#include <stl/types.hpp>
#include <stl/vector.hpp>

namespace kernel::tasks {

struct Thread {
	interrupt::Registers state;
	void* stack = nullptr;
	void* kernel_stack = nullptr;
};

struct Process {
	PhysicalAddress page_table;
	// just one for now
	Thread thread;

	paging::PageTableEntry* get_page_entries();
};

Process allocate_process(void* entry_point);

// should only be called once
Process allocate_kernel_process(void* entry_point);

}