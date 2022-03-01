#include "alloc.hpp"
#include "kernel/serial.hpp"

namespace alloc {
	constexpr auto MEM_SIZE = 0xF40000;
	constexpr auto CHUNKS = 1024;
	// TODO: make a page frame allocator already
	const void* memory = reinterpret_cast<void*>(0x04000000);

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
		return;
		serial(" -- dump info -- \n"_sv);
		size_t count = 0;
		for (MemChunk* chunk = start; chunk != nullptr; chunk = chunk->next) {
			serial("{}\n", *chunk);
			if (++count > chunks_size) {
				serial("uh ok the linked list is broken\n");
				break;
			}
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
				// serial("merging {} with this one {}\n", *chunk, *chunk->next);
				chunk->size += chunk->next->size;

				// chunk->next->size = 666;

				const int index = chunk->next - chunks;
				chunk->next = chunk->next->next;
				if (chunk->next)
					chunk->next->previous = chunk;
				for (size_t i = index; i < chunks_size - 1; ++i) {
					const auto& target = chunks[i + 1];
					if (target.previous)
						--target.previous->next;
					else
						--start;
					if (target.next)
						--target.next->previous;
					chunks[i] = chunks[i + 1];
				}
				--chunks_size;
				if (chunk - chunks > index)
					--chunk;
			}
		}
		dump_info();
	}

	static constexpr auto INVALID = NumberLimit<uptr>::max;

	uptr add_chunk(size_t size) {
		// TODO: maybe instead of the first match try to find the closest in size?
		// or even try to find exact size match, if not then find the least close match
		// thus minimizing small unused chunks
		serial(" - trying to allocate {} bytes\n", size);
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
			serial(" - freeing {} bytes {x}\n", chunk->size, offset);
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
		format_to(write, "chunk[{}]({x},size={},prev={},next={},{})",
			&chunk - alloc::chunks,
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
