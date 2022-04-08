#pragma once

#include <lib/string.hpp>
#include <lib/vector.hpp>
#include <lib/result.hpp>

Result<Vector<u8>> assemble(const StringView& code);
