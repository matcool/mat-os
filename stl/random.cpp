#include "random.hpp"

namespace STL_NS::random {

static constexpr u64 DEFAULT_STATE = 0x853c49e6748fea9b;
static constexpr u64 DEFAULT_INC = 0xda3e39cb94b95bdb;

PCGenerator::PCGenerator(u64 seed) {
	m_inc = DEFAULT_INC;
	this->next_value();
	m_state += seed;
	this->next_value();
}

PCGenerator::PCGenerator() : m_state(DEFAULT_STATE), m_inc(DEFAULT_INC) {}

u32 PCGenerator::next_value() {
	// from PCG's code, magic
	const u64 old_state = m_state;
	m_state = old_state * 0x5851f42d4c957f2d + m_inc;
	const u32 xor_shifted = ((old_state >> 18u) ^ old_state) >> 27u;
	const u32 rot = old_state >> 59u;
	return (xor_shifted >> rot) | (xor_shifted << ((-rot) & 31));
}

u32 PCGenerator::range(u32 end) {
	return range(0, end);
}

u32 PCGenerator::range(u32 start, u32 end) {
	// assumes end > start
	const u32 size = end - start;

	// from PCG's code
	const u32 threshold = -size % size;

	while (true) {
		const auto value = next_value();
		if (value >= threshold) {
			return (value % size) + start;
		}
	}
}

}