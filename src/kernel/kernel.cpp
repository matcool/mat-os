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

#if defined(__linux__) || !defined(__i386__)
	#error "Compilation options are incorrect"
#endif

struct MultibootInfo {
	u32 flags;
	u32 mem_lower;
	u32 mem_upper;
	u32 boot_device;
	u32 cmdline;
	u32 mods_count;
	u32 mods_addr;
	u32 syms[4];
	u32 mmap_length;
	u32 mmap_addr;
	u32 drives_length;
	u32 drives_addr;
	u32 config_table;
	u32 boot_loader_name;
	u32 apm_table;

	u32 vbe_control_info;
	u32 vbe_mode_info;
	u16 vbe_mode;
	u16 vbe_interface_seg;
	u16 vbe_interface_off;
	u16 vbe_interface_len;

	u64 framebuffer_addr;
	u32 framebuffer_pitch;
	u32 framebuffer_width;
	u32 framebuffer_height;
	u8 framebuffer_bpp;
	u8 framebuffer_type;
	union {
		struct {
			u32 framebuffer_palette_addr;
			u32 framebuffer_palette_num_colors;
		} indexed;
		struct {
			u8 framebuffer_red_field_position;
			u8 framebuffer_red_mask_size;
			u8 framebuffer_green_field_position;
			u8 framebuffer_green_mask_size;
			u8 framebuffer_blue_field_position;
			u8 framebuffer_blue_mask_size;
		} rgb;
	} framebuffer_color;
};

extern "C" void kernel_main(MultibootInfo* multiboot) {
	serial_init();

	// paging_init();

	gdt_init();

	pic_init();

	keyboard_init();

	idt_init();

	serial("multiboot info:\nflags: {x}\naddr: {x}\nwidth: {}\nheight: {}\n"_sv,
		multiboot->flags,
		u32(multiboot->framebuffer_addr), multiboot->framebuffer_width, multiboot->framebuffer_height);
	if (multiboot->framebuffer_type == 1) {
		const auto pixels = reinterpret_cast<u32*>(u32(multiboot->framebuffer_addr));
		for (size_t y = 0; y < multiboot->framebuffer_height; ++y) {
			for (size_t x = 0; x < multiboot->framebuffer_width; ++x) {
				pixels[y * multiboot->framebuffer_width + x] = 0xFF223344;
			}
		}
		terminal_init(pixels, multiboot->framebuffer_width, multiboot->framebuffer_height);
	}

	// asm volatile("int3" :);

	// return;
	serial("hello\n"_sv);

	terminal("Check out this awesome font!\nabcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ\n0123456789\n!\"#$%&'()*+?<>= 0 | 1\n"_sv);


	terminal("Hello, kernel World!\n"_sv);
	terminal("I am mat\n"_sv);
	terminal("I am mat 2"_sv);
	terminal("its me mat once again\n"_sv);

	// asm volatile("int3" :);
	// serial("after int 3\n"_sv);

	// asm volatile("int3" :);
	// serial("another int 3\n"_sv);

	// return;

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
