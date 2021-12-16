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
		// TODO: maybe change these to u16 and have it as the direct
		// index into the list, thus saving 4 bytes
		MemChunk* previous;
		MemChunk* next;
		bool used;
	};
	static MemChunk chunks[CHUNKS] = { {
		.offset = 0,
		.size = MEM_SIZE,
		.previous = nullptr,
		.next = nullptr,
		.used = false
	}, {} };
	static size_t chunks_size = 1;
	static MemChunk* start = &chunks[0];

	// TODO: optimization

	void dump_info() {
		serial(" -- dump info -- \n"_sv);
		for (MemChunk* chunk = start; chunk != nullptr; chunk = chunk->next) {
			serial("{}\n", *chunk);
		}
		serial("["_sv);
		for (size_t i = 0; i < chunks_size; ++i) {
			serial("{}, ", chunks[i]);
		}
		serial("]\n -- dump info over -- \n"_sv);
	}

	void merge_chunks() {
		for (MemChunk* chunk = start; chunk != nullptr; chunk = chunk->next) {
			// only merge unused chunks (for now?)
			if (chunk->used) continue;
			while (chunk->next && !chunk->next->used) {
				serial("merging {} with this one {}\n", *chunk, *chunk->next);
				chunk->size += chunk->next->size;
				const auto next = chunk->next->next;

				// chunk->next->size = 666;

				const int index = chunk->next - chunks;
				chunk->next = next;
				for (size_t i = index; i < chunks_size - 1; ++i) {
					const auto& target = chunks[i + 1];
					if (target.previous)
						--target.previous->next;
					else
						--start;
					if (target.next && target.next)
						--target.next->previous;
					chunks[i] = chunks[i + 1];
				}
				--chunks_size;
				if (chunk - chunks > index)
					--chunk;

				if (chunk->next)
					chunk->next->previous = chunk;
			}
		}
		dump_info();
	}

	static constexpr auto INVALID = NumberLimit<uptr>::max;

	uptr add_chunk(size_t size) {
		// TODO: maybe instead of the first match try to find the closest in size?
		// or even try to find exact size match, if not then find the least close match
		// thus minimizing small unused chunks
		serial(" - trying to allocate {}\n", size);
		for (MemChunk* chunk = start; chunk != nullptr; chunk = chunk->next) {
			if (!chunk->used && chunk->size >= size) {
				if (chunk->size == size) {
					chunk->used = true;
					return chunk->offset;
				} else {
					auto& new_chunk = chunks[chunks_size++];
					new_chunk = { .offset = chunk->offset, .size = size, .previous = nullptr, .next = nullptr, .used = true };
					if (chunk->previous == nullptr)
						start = &new_chunk;
					else
						chunk->previous->next = &new_chunk;
					new_chunk.previous = chunk->previous;
					chunk->previous = &new_chunk;
					new_chunk.next = chunk;

					chunk->offset += size;
					chunk->size -= size;
					merge_chunks();
					return new_chunk.offset;
				}
			}
		}
		return INVALID;
	}

	MemChunk* chunk_for_address(uptr addr) {
		for (MemChunk* chunk = start; chunk != nullptr; chunk = chunk->next) {
			if (chunk->offset == addr) return chunk;
		}
		return nullptr;
	}

	void remove_chunk(uptr offset) {
		const auto chunk = chunk_for_address(offset);
		if (chunk) {
			serial(" - setting offset={} to unused\n", chunk->offset);
			chunk->used = false;
			merge_chunks();
		} else {
			serial("improper free? ({x})\n", offset);
		}
	}
}

template <>
struct Formatter<alloc::MemChunk> {
	static void format(FuncPtr<void(char)> write, const alloc::MemChunk& chunk, const StringView&) {
		// lmao
		format_to(write, "chunk({},size={},prev={},next={},{})",
			chunk.offset, chunk.size, chunk.previous ? chunk.previous - alloc::chunks : -1,
			chunk.next ? chunk.next - alloc::chunks : -1, chunk.used ? "used"_sv : "unused"_sv);
	}
};

void* malloc(size_t size) {
	if (size == 0) return nullptr;
	const auto addr = alloc::add_chunk(size);
	if (addr != alloc::INVALID)
		return reinterpret_cast<void*>(reinterpret_cast<uptr>(alloc::memory) + addr);
	return nullptr;
}

void free(void* addr) {
	serial("freeing {}\n", addr);
	alloc::remove_chunk(reinterpret_cast<uptr>(addr) - reinterpret_cast<uptr>(alloc::memory));
}

void* operator new(size_t size) {
	return malloc(size);
}

void* operator new[](size_t size) {
	return operator new(size);
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
