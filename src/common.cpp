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
	// 16mb
	constexpr auto MEM_SIZE = 16000000;
	constexpr auto CHUNKS = 1024;
	static u8 memory[MEM_SIZE];
	struct MemChunk {
		uptr offset;
		size_t size;
		bool used;
	};
	static MemChunk chunks[CHUNKS] = { { .offset = 0, .size = MEM_SIZE, .used = false } };
	static size_t chunks_size = 1;

	// TODO: optimization

	void dump_info() {
		serial_put_hex(reinterpret_cast<uptr>(&memory)); serial_put_char('\n');
		size_t total = 0, available = 0;
		for (size_t i = 0; i < chunks_size; ++i) {
			if (chunks[i].used)
				total += chunks[i].size;
			else
				available += chunks[i].size;
		}
		serial_put_string("Total allocated memory: "); serial_put_number(total); serial_put_char('\n');
		serial_put_string("Total available memory: "); serial_put_number(available); serial_put_char('\n');
	}

	void merge_chunks() {
		dump_info();
		for (size_t i = 0; i < chunks_size; ++i) {

		}
	}

	static constexpr auto INVALID = NumberLimit<uptr>::max;

	uptr add_chunk(size_t size) {
		// TODO: maybe instead of the first match try to find the closest in size?
		// or even try to find exact size match, if not then find the least close match
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
	const auto addr = alloc::add_chunk(size);
	if (addr != alloc::INVALID)
		return reinterpret_cast<void*>(reinterpret_cast<uptr>(&alloc::memory) + addr);
	return nullptr;
}

void free(void* addr) {
	alloc::remove_chunk(reinterpret_cast<uptr>(addr) - reinterpret_cast<uptr>(&alloc::memory));
}