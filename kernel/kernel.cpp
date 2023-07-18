#include <stl/types.hpp>
#include <stl/format.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/serial.hpp>
#include <kernel/idt.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/memory/paging.hpp>
#include <kernel/device/pic.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/screen/framebuffer.hpp>

using namespace kernel;

extern "C" void _start() {
	serial::init();

	serial::put("Hello\n");

	kdbgln("Regular 50: {}, Hex -50: {:#x}, Regular 10: {}", 50, -50, 10);
	kdbgln("does it work? {} {}", true, "i guess");

	idt::init();

	paging::init();

	alloc::init();

	void* page = alloc::allocate_page();
	void* page2 = alloc::allocate_pages(1);
	reinterpret_cast<u64*>(page2)[0] = 0x69696969;
	reinterpret_cast<u64*>(page2)[2] = 0x4201337;

	kdbgln("addr of page: {}", page);
	kdbgln("another page: {}", page2);

	// should fail
	alloc::free_page(page);
	// should work since its the top most page
	alloc::free_page(page2);
	// crash
	// reinterpret_cast<u64*>(page2)[0] = 0x69696969;

	page = alloc::allocate_page();
	kdbgln("new page: {}", page);
	void* page3 = alloc::allocate_page();
	kdbgln("page 3: {}", page3);

	pic::init();
	ps2::init();

	framebuffer::init();

	kdbgln("Finished, halting");
	// halt without disabling interrupts
	while (true) {
		asm volatile("hlt");
	}
}
