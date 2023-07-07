#pragma once

#include <stl/types.hpp>

namespace kernel {

static constexpr usize PAGE_SIZE = 4096;

namespace alloc {

void init();

}

}