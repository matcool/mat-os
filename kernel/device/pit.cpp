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

u64 tick_counter = 0;

void kernel::pit::handle_interrupt() {
	++tick_counter;
	pic::send_eoi(0);
}

void kernel::sleep(u32 ms) {
	const auto start = tick_counter;
	while (tick_counter - start < ms) {
		asm volatile("nop");
	}
}

u64 kernel::pit::get_ticks() {
	return tick_counter;
}

void kernel::pit::init() {
	// channel 0, low then high byte, square wave mode, not bcd
	outb(PIT_COMMAND_PORT, 0b00'11'010'0);
	outb(PIT_CHANNEL0_PORT, CLOCK_DIVISOR & 0xFF);
	outb(PIT_CHANNEL0_PORT, CLOCK_DIVISOR >> 8);

	pic::set_irq_mask(0, true);
}