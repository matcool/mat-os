; Declare constants for the multiboot header.

; align loaded modules on page boundaries
%define ALIGN 1
%define MEMINFO 2
%define VIDEOINFO 4
%define FLAGS ALIGN | MEMINFO | VIDEOINFO
%define MAGIC 0x1BADB002
; checksum of above, to prove we are multiboot
%define CHECKSUM -(MAGIC + (FLAGS))


section .multiboot
align 4
dd MAGIC
dd (FLAGS)
dd CHECKSUM

; unused fields since we are on elf
dd 0
dd 0
dd 0
dd 0
dd 0

; mode type
dd 0
; width
dd 1024
; height
dd 768
; bpp
dd 32


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
	cli

	push ebx

	call kernel_main

loop:
	hlt
	jmp loop
