#include "gdt.hpp"
#include "serial.hpp"

static struct {
	u16 size;
	uptr addr;
} PACKED gdt_register;

// FIXME: maybe rename to segment?
// this is so dumb
struct GDTEntry {
	u16 limit_lower;
	u16 base_lower;
	u8 base_middle;
	u8 access;
	u8 limit_high : 4;
	u8 flags : 4;
	u8 base_high;

	GDTEntry(u32 base, u32 limit, u8 access, u8 flags) : access(access), flags(flags) {
		base_lower = base & 0xFFFF;
		base_middle = (base >> 16) & 0xFF;
		base_high = (base >> 24) & 0xFF;

		limit_lower = limit & 0xFFFF;
		limit_high = (limit >> 16) & 0xF;
	}
} PACKED;

static_assert(sizeof(GDTEntry) == 8, "Zoink");

// access bits                         10011010
constexpr auto GDT_SEGMENT         = 0b10000000;
constexpr auto GDT_CODE_OR_DATA    = 0b00010000;
constexpr auto GDT_EXECUTABLE      = 0b00001000;
constexpr auto GDT_DATA            = 0;
constexpr auto GDT_DC              = 0b00000100;
constexpr auto GDT_RW              = 0b00000010;
// flag bits
constexpr auto GDT_GRANULAR = 0b1000;
constexpr auto GDT_SIZE     = 0b0100;
constexpr auto GDT_LONG     = 0b0010;

static GDTEntry gdt_table[3] = {
	GDTEntry(0, 0, 0, 0),
	// code segment
	GDTEntry(0, 0xFFFFF, GDT_SEGMENT | GDT_CODE_OR_DATA | GDT_RW | GDT_EXECUTABLE, GDT_GRANULAR | GDT_SIZE),
	// data segment
	GDTEntry(0, 0xFFFFF, GDT_SEGMENT | GDT_CODE_OR_DATA | GDT_RW | GDT_DATA,       GDT_GRANULAR | GDT_SIZE)
};

void gdt_init() {
	gdt_register.size = sizeof(gdt_table) - 1;
	gdt_register.addr = reinterpret_cast<uptr>(gdt_table);

	asm volatile("lgdt %0" : : "m"(gdt_register));

	struct {
		u16 size;
		uptr addr;
	} PACKED data;

	asm volatile("sgdt %0" : : "m"(data));

	if (data.size != sizeof(gdt_table) - 1) {
		serial_put_string("wadafak happened\n");
	}
	if (data.addr != reinterpret_cast<uptr>(gdt_table)) {
		serial_put_string("not equal to this\n");
	}
	if (data.addr != reinterpret_cast<uptr>(&gdt_table)) {
		serial_put_string("not equal to this either\n");
	}

	serial_put_string("GDT: ");

	serial_put_number(data.size);
	serial_put_char(' ');
	serial_put_hex(data.addr);
	serial_put_string("\n");
}