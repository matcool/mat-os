#include <kernel/device/pic.hpp>
#include <kernel/device/pit.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/idt.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/memory/paging.hpp>
#include <kernel/screen/framebuffer.hpp>
#include <kernel/serial.hpp>
#include <kernel/tasks/scheduler.hpp>

using namespace kernel;

#include <limine/limine.h>

static volatile limine_module_request module_request = {
	.id = LIMINE_MODULE_REQUEST,
	.revision = 0,
	.response = nullptr,
};

extern "C" void kernel_init() {
	serial::init();

	kdbgln("Booting up...");

	interrupt::init();

	paging::init();

	alloc::init();

	pic::init();
	ps2::init();
	pit::init();

	framebuffer::init();

	kdbgln("Finished initialization");

	if (module_request.response) {
		for (usize i = 0; i < module_request.response->module_count; ++i) {
			auto* mod = module_request.response->modules[i];
			kdbgln("{i:02} - {} with cmd {} at addr {}", i, mod->path, mod->cmdline, mod->address);
		}
	}

	tasks::Scheduler::get().init();

	halt(false);
}
