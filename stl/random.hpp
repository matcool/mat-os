#pragma once

#include "stl.hpp"
#include "types.hpp"

namespace STL_NS::random {

// PCG random number generator, from https://www.pcg-random.org/
class PCGenerator {
	u64 m_state = 0, m_inc = 0;

public:
	PCGenerator(u64 seed);
	PCGenerator();

	// Gets the next value in the sequence.
	u32 next_value();

	// Gets a value in range of 0 <= x < end
	u32 range(u32 end);

	// Gets a value in range of start <= x < end
	u32 range(u32 start, u32 end);
};

// The default rng to use.
using Generator = PCGenerator;

// Gets a value in the range of start <= x < end, using a global generator.
inline u32 in_range(u32 start, u32 end) {
	static Generator rng;
	return rng.range(start, end);
}

};