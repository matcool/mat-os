; Declare constants for the multiboot header.
; align loaded modules on page boundaries
%define ALIGN 0
; provide memory map
%define MEMINFO 1
; this is the Multiboot 'flag' field
%define FLAGS ALIGN | MEMINFO
; 'magic number' lets bootloader find the header
%define MAGIC 0x1BADB002
; checksum of above, to prove we are multiboot
%define CHECKSUM -(MAGIC + FLAGS)


section .multiboot
align 4
dd MAGIC
dd FLAGS
dd CHECKSUM

section .bss
align 16
stack_bottom:
; 16 KiB
times 16384	resb 0
stack_top:

extern kernel_main

section .text
global _start
_start:

	mov [rel stack_top], esp

	call kernel_main

	cli

loop:
	hlt
	jmp loop


global load_idt
load_idt:
	mov eax, [esp + 4]
	lidt [eax]
	ret