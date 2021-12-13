#include "common.hpp"
#include "kernel/serial.hpp"
#include <limits.hpp>

// this is sort of my libc

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void* memcpy(void* dst, const void* src, size_t len) {
	// TODO: optimize this
	auto d = (u8*)dst;
	auto s = (const u8*)src;
	while (len--) {
		*d++ = *s++;
	}
	return dst;
}

void memset(void* dst, u8 value, size_t len) {
	auto d = reinterpret_cast<u8*>(dst);
	while (len--)
		*d++ = value;
}

namespace alloc {
	constexpr auto MEM_SIZE = 0xF40000;
	constexpr auto CHUNKS = 1024;
	// Lol, lets hope nothing is at that addr
	const void* memory = reinterpret_cast<void*>(0x04000000);
	struct MemChunk {
		uptr offset;
		size_t size;
		bool used;
	};
	static MemChunk chunks[CHUNKS] = { { .offset = 0, .size = MEM_SIZE, .used = false }, {} };
	static size_t chunks_size = 1;

	// TODO: optimization

	void dump_info() {
		size_t total = 0, available = 0;
		for (size_t i = 0; i < chunks_size; ++i) {
			const auto& chunk = chunks[i];
			if (chunks[i].used)
				total += chunk.size;
			else
				available += chunk.size;
			serial("Chunk:\n  - offset: {}\n  - size: {}\n  - used: {}\n", chunk.offset, chunk.size, chunk.used);
		}
		// should be ok to have this here as it shouldnt cause any allocations
		serial("Allocated: {x}\nAvailable: {x}\n", total, available);
	}

	void merge_chunks() {
		// only merge non used chunks because merging used chunks can create this situation
		// a = malloc(4)
		// Chunks: {offset = 0, size = 4}
		// b = malloc(4);
		// Chunks: {offset = 0, size = 4}, {offset = 4, size = 4}
		// After merge:
		// Chunks: {offset = 0, size = 8}
		// free(b); -> ?? no offset matches b
		// or
		// free(a); -> The whole chunk gets freed, indirectly freeing b
		// Chunks: {}
		for (size_t i = 0; i < chunks_size; ++i) {
			if (chunks[i].used) continue;
			const auto target_offset = chunks[i].offset + chunks[i].size;
			for (size_t j = 0; j < chunks_size; ++j) {
				if (!chunks[j].used && chunks[j].offset == target_offset) {
					serial("Merging chunk {} with {}\n", j, i);
					chunks[i].size += chunks[j].size;
					if (j < i)
						--i;
					for (size_t k = j + 1; j < chunks_size; ++j)
						chunks[k - 1] = chunks[k];
					--chunks_size;
				}
			}
		}
		dump_info();
	}

	static constexpr auto INVALID = NumberLimit<uptr>::max;

	uptr add_chunk(size_t size) {
		// TODO: maybe instead of the first match try to find the closest in size?
		// or even try to find exact size match, if not then find the least close match
		// thus minimizing small unused chunks
		for (size_t i = 0; i < chunks_size; ++i) {
			auto& chunk = chunks[i];
			if (!chunk.used && chunk.size >= size) {
				if (chunk.size == size) {
					chunk.used = true;
					return chunk.offset;
				} else {
					auto& ret = chunks[chunks_size++] = { .offset = chunk.offset, .size = size, .used = true };
					chunk.offset += size;
					chunk.size -= size;
					merge_chunks();
					return ret.offset;
				}
			}
		}
		return INVALID;
	}

	size_t index_for_address(uptr addr) {
		for (size_t i = 0; i < chunks_size; ++i) {
			if (chunks[i].offset == addr) return i;
		}
		return NumberLimit<size_t>::max;
	}

	void remove_chunk(uptr offset) {
		const auto index = index_for_address(offset);
		if (index != NumberLimit<size_t>::max) {
			chunks[index].used = false;
			merge_chunks();
		} else {
			serial_put_string("huh\n");
		}
	}
}

void* malloc(size_t size) {
	if (size == 0) return nullptr;
	const auto addr = alloc::add_chunk(size);
	if (addr != alloc::INVALID)
		return reinterpret_cast<void*>(reinterpret_cast<uptr>(alloc::memory) + addr);
	return nullptr;
}

void free(void* addr) {
	alloc::remove_chunk(reinterpret_cast<uptr>(addr) - reinterpret_cast<uptr>(alloc::memory));
}

void* operator new(size_t size) {
	return malloc(size);
}

void* operator new(size_t, void* ptr) {
	return ptr;
}

void operator delete(void* ptr) {
	free(ptr);
}

void operator delete[](void* ptr) {
	free(ptr);
}
