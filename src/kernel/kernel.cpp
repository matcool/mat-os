#include "kernel.hpp"
#include "terminal.hpp"
#include "stdio.hpp"
#include "serial.hpp"
#include "idt.hpp"
#include "pic.hpp"
#include "gdt.hpp"

// Check if the compiler thinks you are targeting the wrong operating system.
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

// This tutorial will only work for the 32-bit ix86 targets.
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

__attribute__((interrupt))
void keyboard_interrupt(InterruptFrame*) {
	auto scancode = inb(0x60);
	terminal_put_char(scancode);
	pic_eoi(1);
}

extern "C" void kernel_main() {
	terminal_init();

	serial_init();

	gdt_init();

	pic_init();

	idt_get_table()[0x21] = IDTEntry(&keyboard_interrupt, IDT_GATE | IDT_GATE_INTERRUPT, 0x08);

	idt_init();

	serial_put_string("hello\n");

	terminal_set_color(2);

	printf("Hello, kernel World!\n");
	printf("I am mat\n");
	printf("I am mat 2");
	printf("its me mat once again");


	asm volatile("int3" :);
	serial_put_string("after int 3\n");

	asm volatile("int3" :);
	serial_put_string("another int 3\n");
}
