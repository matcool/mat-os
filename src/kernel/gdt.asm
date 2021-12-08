section .data
gdt_ptr:
	dw 0 ; For limit storage
	dd 0 ; For base storage

section .code
global set_gdt
set_gdt:
	mov eax, [esp + 4]
	mov [gdt_ptr + 2], eax
	mov ax, [esp + 8]
	dec ax
	mov [gdt_ptr], ax
	lgdt [gdt_ptr]
	ret