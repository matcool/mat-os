#include <stl/math.hpp>
#include <kernel/device/pic.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/log.hpp>
#include <kernel/intrinsics.hpp>

using stl::math::get_bit;

static void io_wait() {
	// does nothing
	outb(0x80, 0);
}

using namespace kernel::ps2;

// The output buffer must be full before reading
// from the data port.
bool is_output_buffer_full() {
	return inb(PS2_COM_PORT) & 0b1;
}

// The input buffer must be empty before writing
// to either the data or command ports.
bool is_input_buffer_full() {
	return get_bit(inb(PS2_COM_PORT), 1);
}

// Waits for `getter()` to be true
template <class Func>
void wait_for(Func getter) {
	for (int tries = 0; tries < 100; ++tries) {
		if (getter()) return;
		io_wait();
	}
	panic("Failed wait_for");
}

void kernel::ps2::write(u8 command) {
	wait_for([] { return !is_input_buffer_full(); });
	outb(PS2_COM_PORT, command);
}

void kernel::ps2::write_data(u8 command) {
	wait_for([] { return !is_input_buffer_full(); });
	outb(PS2_DATA_PORT, command);
}

u8 kernel::ps2::read() {
	// the bit i need isnt being set :(
	wait_for([] { return is_output_buffer_full(); });
	return inb(PS2_DATA_PORT);
}

void kernel::ps2::init() {
	init_keyboard();
	init_mouse();

	kdbgln("PS/2 devices initialized");
}