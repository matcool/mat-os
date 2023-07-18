#pragma once

#include <stl/types.hpp>

namespace kernel {

// offset at which the PIC's IRQs get turned into regular interrupts
// so IRQ 1 will be interrupt `PIC_IRQ_OFFSET + 1`
static constexpr u8 PIC_IRQ_OFFSET = 0x20;

static constexpr u16 PIC1_COM_PORT = 0x20;
static constexpr u16 PIC1_DATA_PORT = PIC1_COM_PORT + 1;
static constexpr u16 PIC2_COM_PORT = 0xA0;
static constexpr u16 PIC2_DATA_PORT = PIC2_COM_PORT + 1;

namespace pic {

void init();

// Masks a given IRQ to be either enabled or disabled
void set_irq_mask(u8 irq_index, bool enabled);

// Sends an End Of Interrupt signal, required at the end of IRQs.
void send_eoi(u8 irq);

}

}