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
	printf("its me mat once again");


	asm volatile("int3" :);
	serial_put_string("after int 3\n");

	asm volatile("int3" :);
	serial_put_string("another int 3\n");

	char* a = (char*)malloc(8);
	if (a != nullptr) {
		a[0] = 'H';
		a[1] = 'e';
		a[2] = 'l';
		a[3] = 'l';
		a[4] = 'o';

		free(a);
	} else {
		serial_put_string("malloc failed\n");
	}

}
