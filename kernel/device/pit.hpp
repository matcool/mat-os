#pragma once

#include <stl/types.hpp>

namespace kernel {

namespace pit {

void init();

void handle_interrupt();

}

// Sleeps for a set number of milliseconds.
void sleep(u32 ms);

}