#include <stl/math.hpp>
#include <kernel/device/pic.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>

static constexpr u8 ICW1_ICW4 = 0x01; // Indicates that ICW4 will be present
static constexpr u8 ICW1_INIT = 0x10; // Initialization - required!
static constexpr u8 ICW4_8086 = 0x01; // 8086/88 (MCS-80/85) mode

static void io_wait() {
	// does nothing
	outb(0x80, 0);
}

using namespace kernel;

static void remap_pic(u8 offset1, u8 offset2) {
	const auto mask1 = inb(PIC1_DATA_PORT);
	const auto mask2 = inb(PIC2_DATA_PORT);

	static constexpr auto send_command = [](u8 which, u8 byte) {
		outb(which == 1 ? PIC1_COM_PORT : PIC2_COM_PORT, byte);
		io_wait();
	};

	static constexpr auto send_data = [](u8 which, u8 byte) {
		outb(which == 1 ? PIC1_DATA_PORT : PIC2_DATA_PORT, byte);
		io_wait();
	};

	// starts the initialization sequence (in cascade mode)
	send_command(1, ICW1_INIT | ICW1_ICW4);
	send_command(2, ICW1_INIT | ICW1_ICW4);
	
	// ICW2: PIC1 vector offset
	send_data(1, offset1);
	// ICW2: PIC2 vector offset
	send_data(2, offset2);
	// ICW3: tell PIC1 that there is a PIC2 at IRQ2
	send_data(1, 0b100);
	// ICW3: tell PIC2 its cascade identity
	send_data(2, 0b10);

	// ICW4: have the PICs use 8086 mode (and not 8080 mode)
	send_data(1, ICW4_8086);
	send_data(2, ICW4_8086);

	outb(PIC1_DATA_PORT, mask1);
	outb(PIC2_DATA_PORT, mask2);
}

// masks a given IRQ to be either enabled or disabled
void kernel::pic::set_irq_mask(u8 irq_index, bool enabled) {
	u16 port;
	if (irq_index < 8) {
		port = PIC1_DATA_PORT;
	} else {
		port = PIC2_DATA_PORT;
		irq_index -= 8;
	}
	auto mask = inb(port);
	mat::math::set_bit(mask, irq_index, !enabled);
	outb(port, mask);
}

void kernel::pic::init() {
	remap_pic(PIC_IRQ_OFFSET, PIC_IRQ_OFFSET + 8);

	kdbgln("PIC initialized");
}