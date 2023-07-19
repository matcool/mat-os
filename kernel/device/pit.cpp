#include <kernel/device/pit.hpp>
#include <kernel/device/pic.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>

static constexpr u32 PIT_CLOCK_HZ = 1.193182 * 1'000'000;
static constexpr u16 PIT_CHANNEL0_PORT = 0x40;
static constexpr u16 PIT_COMMAND_PORT = 0x43;

// frequency at which the interrupts will be triggered
static constexpr u32 FREQ_HZ = 1000;
static constexpr u32 CLOCK_DIVISOR = PIT_CLOCK_HZ / FREQ_HZ;
static_assert(CLOCK_DIVISOR <= 65536, "Invalid divisor for the PIT");

u32 counter = 0;

void kernel::pit::handle_interrupt() {
	++counter;
	if (counter == FREQ_HZ) {
		counter = 0;
	}
	pic::send_eoi(0);
}

void kernel::pit::init() {
	// channel 0, low then high byte, square wave mode, not bcd
	outb(PIT_COMMAND_PORT, 0b00'11'010'0);
	outb(PIT_CHANNEL0_PORT, CLOCK_DIVISOR & 0xFF);
	outb(PIT_CHANNEL0_PORT, CLOCK_DIVISOR >> 8);

	pic::set_irq_mask(0, true);
}