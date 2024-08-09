#pragma once

#include <stl/types.hpp>

namespace kernel::gdt {

static constexpr int KERNEL_CODE_SEGMENT = 5 * 8;
static constexpr int KERNEL_DATA_SEGMENT = 6 * 8;

static constexpr int USER_CODE_SEGMENT = 7 * 8;
static constexpr int USER_DATA_SEGMENT = 8 * 8;

void init();

}