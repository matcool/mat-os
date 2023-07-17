#pragma once

#include <stl/types.hpp>

namespace kernel {

// offset at which the PIC's IRQs get turned into regular interrupts
// so IRQ 1 will be interrupt `PIC_IRQ_OFFSET + 1`
static constexpr u8 PIC_IRQ_OFFSET = 0x20;

namespace ps2 {

void init();

}

}