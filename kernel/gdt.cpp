#include <kernel/gdt.hpp>
#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>

using namespace kernel::gdt;

static struct [[gnu::packed]] {
	u16 size;
	void* addr;
} gdt_register;

struct [[gnu::packed]] GDTEntry {
	u16 limit_lower;
	u16 base_lower;
	u8 base_middle;
	u8 access;
	u8 limit_high : 4;
	u8 flags : 4;
	u8 base_high;

	constexpr GDTEntry(u32 base, u32 limit, u8 access, u8 flags) : access(access), flags(flags) {
		base_lower = base & 0xFFFF;
		base_middle = (base >> 16) & 0xFF;
		base_high = (base >> 24) & 0xFF;

		limit_lower = limit & 0xFFFF;
		limit_high = (limit >> 16) & 0xF;
	}
};

// clang-format off

// access bits
enum GDTEntryAccess : u8 {
	GDT_PRESENT       = 0b10000000,

	GDT_RING0         = 0b00000000,
	GDT_RING1         = 0b00100000,
	GDT_RING2         = 0b01000000,
	GDT_RING3         = 0b01100000,

	GDT_SYSTEM        = 0b00000000,
	GDT_CODE_OR_DATA  = 0b00010000,

	GDT_DATA          = 0b00000000,
	GDT_EXECUTABLE    = 0b00001000,

	GDT_DC            = 0b00000100,
	GDT_RW            = 0b00000010,

	GDT_ACCESSED      = 0b00000001,
};

// flag bits
enum GDTEntryFlags : u8 {
	GDT_GRANULAR = 0b1000, // whether to use 4 KiB blocks
	GDT_SIZE     = 0b0100, // whether its a 32 bit segment
	GDT_LONG     = 0b0010, // whether its a 64-bit **code** segment. should not be used with GDT_SIZE
};

// clang-format on

static_assert(sizeof(GDTEntry) == 8);

struct [[gnu::packed]] TaskStateSegment {
	u32 _reserved = 0;
	u64 rsp0 = 0;
	u64 rsp1 = 0;
	u64 rsp2 = 0;
	// rest i dont care about
	u32 _pad[19]{};
};

static_assert(sizeof(TaskStateSegment) == 0x68);

static TaskStateSegment tss_instance{};

struct [[gnu::packed]] GDTSystemEntry {
	u16 limit_low;
	u16 base_low;
	u8 base_mid1;
	u8 access;
	u8 limit_extra : 4;
	u8 flags : 4;
	u8 base_mid2;
	u32 base_high;
	u32 _reserved;

	GDTSystemEntry(u64 base, u32 limit, u8 access, u8 flags) : access(access), flags(flags) {
		base_low = (base & 0x0000'0000'0000'FFFF) >> 0;
		base_mid1 = (base & 0x0000'0000'00FF'0000) >> 16;
		base_mid2 = (base & 0x0000'0000'FF00'0000) >> 24;
		base_high = (base & 0xFFFF'FFFF'0000'0000) >> 32;

		limit_low = limit & 0x00'FFFF;
		limit_extra = (limit & 0x0F'0000) >> 16;
	}
};

static_assert(sizeof(GDTSystemEntry) == 16);

// based off limine's GDT structure
static GDTEntry gdt_table[] = {
	// required null entry
	GDTEntry(0, 0, 0, 0),

	// 16 bit code segment
	GDTEntry(0, 0xffff, GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW | GDT_EXECUTABLE, 0),
	// 16 bit data segment
	GDTEntry(0, 0xffff, GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW | GDT_DATA, 0),

	// 32 bit code segment
	GDTEntry(
		0, 0xffffffff, GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW | GDT_EXECUTABLE, GDT_GRANULAR | GDT_SIZE
	),
	// 32 bit data segment
	GDTEntry(0, 0xffffffff, GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW | GDT_DATA, GDT_GRANULAR | GDT_SIZE),

	// 64 bit code segment
	GDTEntry(0, 0, GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW | GDT_EXECUTABLE, GDT_LONG),
	// 64 bit data segment
	GDTEntry(0, 0, GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW | GDT_DATA, 0),

	// ring 3 code and data segment
	GDTEntry(0, 0, GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW | GDT_EXECUTABLE | GDT_RING3, GDT_LONG),
	GDTEntry(0, 0, GDT_PRESENT | GDT_CODE_OR_DATA | GDT_RW | GDT_DATA | GDT_RING3, 0),

	// TSS entry,
	// which is actually 16 bytes! so add two dummy gdtentries, and set it up later
	GDTEntry(0, 0, 0, 0),
	GDTEntry(0, 0, 0, 0),
};

void init_tss_entry() {
	auto& entry = *reinterpret_cast<GDTSystemEntry*>(&gdt_table[9]);
	entry = GDTSystemEntry(
		reinterpret_cast<u64>(&tss_instance),
		sizeof(TaskStateSegment),
		GDT_PRESENT | GDT_SYSTEM | GDT_EXECUTABLE | GDT_ACCESSED,
		GDT_SIZE
	);

	tss_instance.rsp0 = reinterpret_cast<uptr>(kernel::alloc::allocate_pages(2));
}

void kernel::gdt::init() {
	init_tss_entry();

	gdt_register.addr = (void*)(&gdt_table[0]);
	gdt_register.size = sizeof(gdt_table) - 1;
	asm volatile("lgdt %0;" : : "m"(gdt_register));

	// load the tss
	// tss entry is 9th on the table
	asm volatile("movw %0, %%ax; ltr %%ax" : : "i"(9 * sizeof(GDTEntry)));
}