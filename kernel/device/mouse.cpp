#include <stl/math.hpp>
#include <kernel/device/ps2.hpp>
#include <kernel/device/pic.hpp>
#include <kernel/intrinsics.hpp>
#include <kernel/log.hpp>
#include <kernel/screen/window_manager.hpp>

using stl::math::get_bit;

struct MousePacket {
	u8 status, mov_x, mov_y;
	u8 counter = 0;

	// Returns true if theres not a complete packet yet,
	// meaning the interrupt should return
	bool put_byte(u8 byte) {
		switch (counter++) {
			case 0: status = byte; break;
			case 1: mov_x = byte; break;
			case 2: mov_y = byte; break;
			default: break;
		}
		if (counter == 3) {
			counter = 0;
			return false;
		}
		return true;
	}

	// i cant get the overflow bits to be set.. so this is good enough for now

	i32 x_offset() const {
		return static_cast<i8>(mov_x);
	}

	i32 y_offset() const {
		return -static_cast<i8>(mov_y);
	}

	bool left_button() const { return get_bit(status, 0); }
	bool right_button() const { return get_bit(status, 1); }
} packet;

void kernel::ps2::handle_mouse() {
	if (packet.put_byte(inb(PS2_DATA_PORT))) {
		return pic::send_eoi(12);
	}

	window::WindowManager::get().handle_mouse({ packet.x_offset(), packet.y_offset() }, packet.left_button());

	pic::send_eoi(12);
}

using namespace kernel;

void wait_for_ack() {
	if (auto byte = ps2::read(); byte != 0xFA) [[unlikely]] {
		panic("Mouse did not respond with ACK (0xFA), got {:#02x}", byte);
	}
}

void mouse_command(u8 command) {
	// Tell the PS/2 controller to send data
	// to the second port (the mouse)
	ps2::write(0xD4);
	// Send the command
	ps2::write_data(command);
	wait_for_ack();
}

void kernel::ps2::init_mouse() {
	// Enable the mouse IRQ
	pic::set_irq_mask(12, true);

	// Enable auxiliary device
	ps2::write(0xA8);
	// wait_for_ack();

	// Set defaults
	mouse_command(0xF6);

	// Enable data reporting
	mouse_command(0xF4);

	// temporarily disable keyboard as to not
	// get an interrupt on the next command
	// TODO: figure this out properly
	pic::set_irq_mask(1, false);

	// Get compaq status byte
	// this triggers the keyboard interrupt for some reason :(
	ps2::write(0x20);
	auto status = ps2::read();
	// Enable IRQ12
	status = math::set_bit(status, 1, true);
	// Disable the Disable mouse clock bit
	status = math::set_bit(status, 5, false);
	// Set compaq status byte
	ps2::write(0x60);
	ps2::write_data(status);

	// enable it back
	pic::set_irq_mask(1, true);
}