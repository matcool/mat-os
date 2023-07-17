#include "memory.hpp"

namespace STL_NS {

void memset(void* ptr, u8 value, usize bytes) {
	const u64 value4 = value | (value << 8) | (value << 16) | (value << 24);
	const u64 value8 = value4 | (value4 << 32);
	usize i = 0;
	for (; i + sizeof(u64) <= bytes; i += sizeof(u64)) {
		reinterpret_cast<u64*>(ptr)[i / sizeof(u64)] = value8;
	}
	for (; i < bytes; ++i) {
		reinterpret_cast<u8*>(ptr)[i] = value;
	}
}

}