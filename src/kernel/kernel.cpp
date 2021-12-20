#include "kernel.hpp"
#include "terminal.hpp"
#include "stdio.hpp"
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

#if defined(__linux__) || !defined(__i386__)
	#error "Compilation options are incorrect"
#endif
extern "C" {
	void* __dso_handle;

	// TODO: do this properly Lol
	int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso) {

		return 0;
	}
}

extern "C" void kernel_main(MultibootInfo* multiboot) {
	serial_init();

	paging_init();

	gdt_init();

	pic_init();

	keyboard_init();
	mouse_init();

	idt_init();

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

	serial("-- begin vector test --\n"_sv);
	Vector<int> test;
	for (const auto i : test) { terminal("vec: {}\n", i); }
	test.push_back(1);
	test.push_back(2);
	test.push_back(3);
	for (int& i : test) { terminal("vec: {}\n", i++); }
	for (int i = 0; i < 5; ++i) {
		terminal("adding {}\n", i * 13);
		test.push_back(i * 13);
	}
	for (const auto i : test) { terminal("vec: {}\n", i); }
	serial("-- end vector test --\n"_sv);

	terminal("Check out this hex number {x}, pretty cool right {} {x}\n"_sv, 1337, i8(-127), i8(-127));
	terminal("Hello {}!\n"_sv, "world");

	String str = "hi";
	str.push_back('s');
	str.push_back('u');
	str.push_back('c');
	str.push_back('k');
	terminal("String is \"{}\"\n", str);
	for (int i = 0; i < 20; ++i) {
		str.push_back('A' + i);
	}
	terminal("String is now \"{}\" (size={}, capacity={})\n", str, str.size(), str.capacity());
}
