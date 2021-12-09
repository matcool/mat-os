#include "kernel.hpp"
#include "terminal.hpp"
#include "stdio.hpp"

// Check if the compiler thinks you are targeting the wrong operating system.
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
// This tutorial will only work for the 32-bit ix86 targets.
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

static uint8_t gdt_raw[8][3];

static struct {
	uint16_t limit;
	void* base;
} PACKED gdt_reg;

struct GDTEntry {
	size_t base;
	size_t limit;
	uint8_t type;
};

void gdt_initialize() {
	GDTEntry table[3];

	table[0] = {.base=0, .limit=0, .type=0};               // Selector 0x00 cannot be used
	table[1] = {.base=0, .limit=0xffffffff, .type=0x9A};   // Selector 0x08 will be our code
	table[2] = {.base=0, .limit=0xffffffff, .type=0x92};   // Selector 0x10 will be our data

	// TODO: task state segment entry

	for (size_t i = 0; i < 3; ++i) {
		auto& entry = table[i];
		auto raw = gdt_raw[i];
		if (entry.limit > 65536) {
			entry.limit >>= 12;
			raw[6] = 0xC0;
		} else {
			raw[6] = 0x40;
		}

		raw[0] = entry.limit & 0xFF;
		raw[1] = (entry.limit >> 8) & 0xFF;
		raw[6] |= (entry.limit >> 16) & 0xF;

		raw[2] = entry.base & 0xFF;
		raw[3] = (entry.base >> 8) & 0xFF;
		raw[4] = (entry.base >> 16) & 0xFF;
		raw[7] = (entry.base >> 24) & 0xFF;
	
		raw[5] = entry.type;
	}

	gdt_reg.limit = sizeof(gdt_raw) - 1;
	gdt_reg.base = reinterpret_cast<void*>(gdt_raw);

	asm volatile("lgdt %0" : : "X"(gdt_reg));
}

struct IDTEntry {
	uint16_t isr_low;   // The lower 16 bits of the ISR's address
	uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t reserved;   // Set to zero
	uint8_t attributes; // Type and attributes; see the IDT page
	uint16_t isr_high;  // The higher 16 bits of the ISR's address
} PACKED;

constexpr const auto IDT_ENTRIES = 256;
static IDTEntry idt_table[IDT_ENTRIES];

static struct {
	uint16_t limit;
	uint32_t base;
} PACKED idt_reg;

extern "C" void exception_handler() {
	asm("cli\n"
	    "hlt":);
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
	auto& descriptor = idt_table[vector];

	descriptor.isr_low    = reinterpret_cast<uint32_t>(isr) & 0xFFFF;
	descriptor.kernel_cs  = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
	descriptor.attributes = flags;
	descriptor.isr_high   = reinterpret_cast<uint32_t>(isr) >> 16;
	descriptor.reserved   = 0;
}

extern void* isr_stub_table[];

extern "C" void kb_handler() {
	printf("hey?\n");
}

void idt_initialize() {
	idt_reg.base = reinterpret_cast<uintptr_t>(&idt_table[0]);
	idt_reg.limit = uint16_t(sizeof(IDTEntry) * IDT_ENTRIES - 1);

	for (u8 vector = 0; vector < 32; ++vector) {
		idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
	}

	idt_set_descriptor(0x20 + 0, reinterpret_cast<void*>(&kb_handler), 0x8E);
	idt_set_descriptor(0x20 + 1, reinterpret_cast<void*>(&kb_handler), 0x8E);
	idt_set_descriptor(0x20 + 2, reinterpret_cast<void*>(&kb_handler), 0x8E);
	idt_set_descriptor(0x20 + 3, reinterpret_cast<void*>(&kb_handler), 0x8E);
	idt_set_descriptor(0x20 + 4, reinterpret_cast<void*>(&kb_handler), 0x8E);
	idt_set_descriptor(0x20 + 5, reinterpret_cast<void*>(&kb_handler), 0x8E);

	asm volatile("lidt %0\n"
	             "sti" : : "m"(idt_reg));
	// sti enables interrupts by setting the interrupt flag
}

#define PIC1_CMD 0x20
#define PIC1_DAT 0x21
#define PIC2_CMD 0xA0
#define PIC2_DAT 0xA1

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

#define PIC_READ_IRR 0xA
#define PIC_READ_ISR 0xB

void pic_eoi(u8 irq) {
	if (irq >= 8)
		outb(PIC2_CMD, 0x20);
	outb(PIC1_CMD, 0x20);
}

void pic_remap(u8 pic1_offset, u8 pic2_offset) {
	auto mask1 = inb(PIC1_DAT);
	auto mask2 = inb(PIC2_DAT);

	outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DAT, pic1_offset);
	io_wait();
	outb(PIC2_DAT, pic2_offset);
	io_wait();
	outb(PIC1_DAT, 4);
	io_wait();
	outb(PIC2_DAT, 2);
	io_wait();
 
	outb(PIC1_DAT, ICW4_8086);
	io_wait();
	outb(PIC2_DAT, ICW4_8086);
	io_wait();
 
	outb(PIC1_DAT, mask1);
	outb(PIC2_DAT, mask2);
}

void pic_set_mask(u8 irq) {
	u16 port;

	if (irq < 8) {
		port = PIC1_DAT;
	} else {
		port = PIC2_DAT;
		irq -= 8;
	}
	outb(port, inb(port) | (1 << irq));
}

void pic_clear_mask(u8 irq) {
	u16 port;

	// TODO: not copy paste this mayb
	if (irq < 8) {
		port = PIC1_DAT;
	} else {
		port = PIC2_DAT;
		irq -= 8;
	}
	outb(port, inb(port) & ~(1 << irq));
}

u16 _pic_get_irq_reg(u8 value) {
	outb(PIC1_CMD, value);
	outb(PIC2_CMD, value);
	return (inb(PIC2_CMD) << 8) | inb(PIC1_CMD);
}

u16 pic_get_irr() {
	return _pic_get_irq_reg(PIC_READ_IRR);
}

u16 pic_get_isr() {
	return _pic_get_irq_reg(PIC_READ_ISR);
}

// TODO: Spurious irqs

#define PORT 0x3f8 // COM1

int serial_ininitialize() {
	outb(PORT + 1, 0x00);    // Disable all interrupts
	outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(PORT + 1, 0x00);    //                  (hi byte)
	outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
	outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
	outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
	outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

	// Check if serial is faulty (i.e: not same byte as sent)
	if (inb(PORT + 0) != 0xAE) {
		return 1;
	}

	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	outb(PORT + 4, 0x0F);
	return 0;
}


extern "C" void kernel_main() {
	// Initialize terminal interface
	terminal_initialize();

	gdt_initialize();

	pic_remap(0x20, 0x28);

	idt_initialize();

	if (serial_ininitialize()) {
		printf("fucky wucky\n");
	}

	terminal_set_color(2);
 
	printf("Hello, kernel World!\n");
	printf("I am mat\n");
	printf("I am mat 2");
	printf("its me mat once again");

}
