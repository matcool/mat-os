#include <kernel/device/pic.hpp>
#include <kernel/device/pit.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/gdt.hpp>
#include <kernel/idt.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/memory/paging.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/serial.hpp>
#include <kernel/tasks/scheduler.hpp>

using namespace kernel;

[[gnu::naked]] void test_user_function() {
	// will intentionally GPF at the cli instruction
	asm volatile(R"(
		mov $1, %rax
		mov $2, %rax
		mov $3, %rax
		cli
	)");
}

static auto* myptr = &test_user_function;

constexpr int ring3_code = (7 * 8) | 3;
constexpr int ring3_data = (8 * 8) | 3;

[[gnu::naked]] void jump_usermode() {
	asm volatile(R"(
		cli

		movw %1, %%ax
		movw %%ax, %%ds
		movw %%ax, %%es
		movw %%ax, %%fs
		movw %%ax, %%gs

		mov %%rsp, %%rax
		pushq %1
		pushq %%rax
		pushfq
		pushq %0
		pushq %2
		iretq
	)"
	             :
	             : "i"(ring3_code), "i"(ring3_data), "m"(myptr));
}

extern "C" void kernel_init() {
	serial::init();

	kdbgln("Booting up...");

	interrupt::init();

	paging::init();

	alloc::init();

	gdt::init();

	paging::update_page(
		VirtualAddress((void*)(&test_user_function)),
		paging::PageOptions{
			.executable = true,
			.user = true,
		}
	);

	paging::explore_addr(reinterpret_cast<uptr>(&test_user_function));

	jump_usermode();

	pic::init();
	ps2::init();
	pit::init();

	framebuffer::init();

	kdbgln("Finished initialization");

	tasks::Scheduler::get().init();

	halt(false);
}
