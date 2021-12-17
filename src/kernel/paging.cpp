#include "paging.hpp"
#include "log.hpp"

void set_bit(u32& value, u8 index, bool set) {
	if (set)
		value |= (1 << index);
	else
		value &= ~(1 << index);
}

static u32 page_directory[1024] __attribute__((aligned(4096)));
static u32 tables[1024][1024] __attribute__((aligned(4096)));

void paging_init() {

	u32 addr = 0;
	for (size_t i = 0; i < 1024; ++i) {
		auto& table = tables[i];
		auto& dir = page_directory[i];
		dir = reinterpret_cast<u32>(&table) & 0xFFFFF000;
		set_bit(dir, 7, 0); // PS
		set_bit(dir, 5, 0); // A
		set_bit(dir, 4, 0); // PCD
		set_bit(dir, 3, 0); // PWT (i dont even know what this one does)
		set_bit(dir, 2, 1); // U/S
		set_bit(dir, 1, 1); // R/W
		set_bit(dir, 0, 1); // Present
		for (size_t j = 0; j < 1024; ++j) {
			auto& entry = table[j];
			entry = addr & 0xFFFFF000;
			addr += 4096;
			set_bit(entry, 8, 0); // G
			set_bit(entry, 7, 0); // PAT
			set_bit(entry, 6, 0); // D
			set_bit(entry, 5, 0); // A
			set_bit(entry, 4, 0); // PCD
			set_bit(entry, 3, 0); // PWT
			set_bit(entry, 2, 1); // U/S
			set_bit(entry, 1, 1); // R/W
			set_bit(entry, 0, 1); // Present
		}
	}

	asm volatile(
		"mov %0, %%cr3;"
		"mov %%cr0, %%eax;"
		"or $0x80000001, %%eax;"
		"mov %%eax, %%cr0;"
		: : "r"(page_directory)
	);

	log("Paging initialized"_sv);
}
