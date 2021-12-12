#include "kernel.hpp"
#include "terminal.hpp"
#include "stdio.hpp"
#include "serial.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "gdt.hpp"
#include "keyboard.hpp"
#include "string.hpp"

#if defined(__linux__) || !defined(__i386__)
	#error "Compilation options are incorrect"
#endif


extern "C" void kernel_main() {
	terminal_init();

	serial_init();

	gdt_init();

	pic_init();

	keyboard_init();

	idt_init();

	serial_put_string("hello\n");

	printf("Hello, kernel World!\n");
	printf("I am mat\n");
	printf("I am mat 2");
	printf("its me mat once again\n");


	asm volatile("int3" :);
	serial_put_string("after int 3\n");

	asm volatile("int3" :);
	serial_put_string("another int 3\n");

	char* data = (char*)malloc(8);
	if (data != nullptr) {
		data[0] = 'H';
		data[1] = 'e';
		data[2] = 'l';
		data[3] = 'l';
		data[4] = 'o';
		data[5] = '\n';
		data[6] = 0;

		terminal_write_string(data);

		free(data);
	} else {
		serial_put_string("malloc failed\n");
	}

	terminal("hello {} there! {} + {} = {}\n"_sv, 23, 1, 2, 3);
	terminal("this shouldnt get formatted {{}}, but this should {}\n"_sv, 42);
}
