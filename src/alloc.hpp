#pragma once
#include "common.hpp"

namespace alloc {
	struct MemChunk {
		uptr offset;
		size_t size;
		// TODO: maybe change these to u16 and have it as the direct
		// index into the list, thus saving 4 bytes
		MemChunk* previous;
		MemChunk* next;
		bool used;
	};
}
