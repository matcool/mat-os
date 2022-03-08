#include "kernel.hpp"
#include "terminal.hpp"
#include "serial.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "gdt.hpp"
#include "keyboard.hpp"
#include "string.hpp"
#include "array.hpp"
#include "vector.hpp"
#include "log.hpp"
#include "paging.hpp"
#include "mouse.hpp"
#include "screen.hpp"
#include <lib/hash-containers.hpp>
#include "pit.hpp"
#include "shell.hpp"
#include <lib/variant.hpp>
#include <lib/result.hpp>

#if defined(__linux__) || !defined(__i386__)
	#error "Compilation options are incorrect"
#endif

extern "C" void kernel_main(MultibootInfo* multiboot) {
	serial_init();

	paging_init();

	gdt_init();

	pic_init();

	pit_init();

	keyboard_init();
	mouse_init();

	kernel::InterruptDescriptorTable::init();

	serial("multiboot addr is {}\n", multiboot);
	serial("mem_lower is {}, mem_upper is {}, diff is {}\n", multiboot->mem_lower, multiboot->mem_upper, multiboot->mem_upper - multiboot->mem_lower);

	serial("multiboot info:\nflags: {x}\naddr: {x}\nwidth: {}\nheight: {}\n"_sv,
		multiboot->flags,
		u32(multiboot->framebuffer_addr), multiboot->framebuffer_width, multiboot->framebuffer_height);
	auto& screen = Screen::get();
	if (multiboot->framebuffer_type == 1) {
		const auto pixels = reinterpret_cast<u32*>(u32(multiboot->framebuffer_addr));
		// TODO: not assume bpp and other info
		screen.init(multiboot->framebuffer_width, multiboot->framebuffer_height, pixels);
		terminal_init();
	}
	screen.redraw();

	serial("hello\n"_sv);

	terminal("Check out this awesome font!\nabcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ\n0123456789\n!\"#$%&'()*+?<>= 0 | 1\n"_sv);


	terminal("Hello, kernel World!\n"_sv);
	terminal("I am mat\n"_sv);
	terminal("I am mat 2"_sv);
	terminal("its me mat once again\n"_sv);

	asm volatile("int3" :);
	serial("after int 3\n"_sv);

	asm volatile("int3" :);
	serial("another int 3\n"_sv);

	serial("Going to allocate 8 bytes\n"_sv);
	char* data = (char*)malloc(8);
	if (data != nullptr) {
		data[0] = 'H';
		data[1] = 'e';
		data[2] = 'l';
		data[3] = 'l';
		data[4] = 'o';
		data[5] = '\n';
		data[6] = 0;

		terminal("malloc'd string: {}", data);

		free(data);
	} else {
		serial("malloc failed\n");
	}

	terminal("hello {} there! {} + {} = {}\n"_sv, 23, 1, 2, 3);
	terminal("this shouldnt get formatted {{}} {{hello}} {{ world {} }}, but this should {}\n"_sv, 31, 42);

	terminal("Check out this hex number {x}, pretty cool right {} {x}\n"_sv, 1337, i8(-127), i8(-127));
	terminal("Hello {}!\n"_sv, "world");

	shell_init();

}
