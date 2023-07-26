#include <kernel/log.hpp>
#include <kernel/memory/allocator.hpp>
#include <stl/math.hpp>
#include <stl/memory.hpp>

using math::div_ceil;

using namespace kernel;

// to what address should allocations be aligned to
// essentially the "page size" to the heap allocator, i think
static constexpr usize ALLOC_ALIGN = 16;

// This struct will be present at the beginning of each heap block,
// as to keep track of all the metadata such as size and allocated regions.
struct HeapBlock {
	HeapBlock* next_block = nullptr;
	// size (in bytes) of this entire block
	usize block_size = 0;

	using BitmapType = u8;
	// a bitmap array of the allocated regions.
	// every two bits represents `ALLOC_ALIGN` bytes of memory.
	// Each 2 bits can represent:
	//   00 - Free space
	//   01 - Used space and start of allocation
	//   11 - Used space but rest of allocation
	//   10 - ??
	// For example, "00 01 11 01 01 00" means there are 3 allocations:
	//   at +16 there are 32 bytes allocated
	//   at +48 there are 16 bytes allocated
	//   at +64 there are 16 bytes allocated
	BitmapType allocated[];

	static constexpr usize entries_per_byte = 8 / 2;

	static constexpr u8 BM_FREE_SPACE = 0b00;
	static constexpr u8 BM_ALLOC_START = 0b01;
	static constexpr u8 BM_ALLOC_CONT = 0b11;

	// amount of entries in the bitmap
	usize entries() const {
		// entries = (block_size - sizeof(HeapBlock) - entries_size) / ALLOC_ALIGN
		// entries = (block_size - sizeof(HeapBlock) - (entries / 4)) / ALLOC_ALIGN
		// entries * ALLOC_ALIGN = block_size - sizeof(HeapBlock) - (entries / 4)
		// entries * ALLOC_ALIGN + entries / 4 = block_size - sizeof(HeapBlock)
		// entries * (ALLOC_ALIGN + 1/4) = block_size - sizeof(HeapBlock)
		// entries = (block_size - sizeof(HeapBlock)) / (ALLOC_ALIGN + 1/4)
		// get rid of the 1/4:
		// entries = (block_size - sizeof(HeapBlock)) / ((4*ALLOC_ALIGN + 1) / 4)
		// entries = (block_size - sizeof(HeapBlock)) * 4 / (4 * ALLOC_ALIGN + 1)
		return (block_size - sizeof(HeapBlock)) * entries_per_byte /
			(entries_per_byte * ALLOC_ALIGN + 1);
	}

	// amount of bytes occupied by the bitmap array
	usize entries_size() const { return div_ceil<usize>(entries(), entries_per_byte); }

	// total amount of bytes occupied by the metadata itself
	usize metadata_size() const {
		// round up to alloc_align so that the allocations are.. aligned
		return div_ceil(sizeof(HeapBlock) + entries_size(), ALLOC_ALIGN) * ALLOC_ALIGN;
	}

	static usize calculate_block_size_for_big_alloc(usize alloc_size) {
		// block_size must be big enough so that block_size >= metadata_size + alloc_size
		// so: block_size >= sizeof(HeapBlock) + entries_size + alloc_size
		// block_size >= sizeof(HeapBlock) + (entries / 4) + alloc_size
		// block_size >= sizeof(HeapBlock) + (((block_size - sizeof(HeapBlock)) / ALLOC_ALIGN) / 4)
		// + alloc_size block_size >= sizeof(HeapBlock) + ((block_size - sizeof(HeapBlock)) /
		// (ALLOC_ALIGN * 4)) + alloc_size 4*ALLOC_ALIGN * block_size >= 4*ALLOC_ALIGN *
		// sizeof(HeapBlock) + block_size - sizeof(HeapBlock) + 4*ALLOC_ALIGN * alloc_size 4*ALLOC_ALIGN
		// * block_size - block_size >= 4*ALLOC_ALIGN * sizeof(HeapBlock) - sizeof(HeapBlock) +
		// 4*ALLOC_ALIGN * alloc_size block_size * (4*ALLOC_ALIGN - 1) >= sizeof(HeapBlock) *
		// (4*ALLOC_ALIGN - 1) + 4*ALLOC_ALIGN * alloc_size block_size >= (sizeof(HeapBlock) *
		// (4*ALLOC_ALIGN - 1) + 4*ALLOC_ALIGN * alloc_size) / (4*ALLOC_ALIGN - 1)
		static constexpr auto mult = entries_per_byte * ALLOC_ALIGN;
		return div_ceil((sizeof(HeapBlock) * (mult - 1) + alloc_size * mult) / (mult - 1), PAGE_SIZE) *
			PAGE_SIZE;
	}

	// check if a pointer is contained within this block
	bool contains(void* ptr) const {
		const auto ptr_value = reinterpret_cast<uptr>(ptr);
		const auto this_value = reinterpret_cast<uptr>(this);
		return ptr_value > this_value && ptr_value < this_value + block_size;
	}

	// converts an index into the bitmap to a virtual address (ptr)
	void* index_to_ptr(usize index) const {
		const auto this_addr = VirtualAddress(this);
		return (this_addr + metadata_size() + index * ALLOC_ALIGN).ptr();
	}

	// converts a virtual address to an index into the bitmap,
	// assuming its in this block
	usize ptr_to_index(void* ptr) const {
		const auto ptr_value = reinterpret_cast<uptr>(ptr);
		const auto this_value = reinterpret_cast<uptr>(this);
		return (ptr_value - (this_value + metadata_size())) / ALLOC_ALIGN;
	}

	u8 get_entry(usize index) const {
		const auto offset = (index % entries_per_byte) * 2;
		return allocated[index / entries_per_byte] >> offset & 0b11;
	}

	void set_entry(usize index, u8 value) {
		const auto offset = (index % entries_per_byte) * 2;
		auto& entry = allocated[index / entries_per_byte];
		entry = (entry & ~(BitmapType(0b11) << offset)) | (value << offset);
	}

	// tries to allocate some bytes within this block.
	// if unable to, returns nullptr
	void* try_allocate(usize size) {
		if (size >= block_size) return nullptr;
		const auto size_entries = div_ceil(size, ALLOC_ALIGN);
		for (usize i = 0; i < entries(); ++i) {
			usize j = 0;
			// mm yes very nice search
			for (; j < size_entries; ++j) {
				if (get_entry(i + j) != BM_FREE_SPACE) break;
			}
			// if j didnt reach the end then it didnt find enough space,
			// so skip the entries it looked through
			if (j != size_entries) {
				i += j;
				continue;
			}
			// we found enough space for the allocation!
			set_entry(i, BM_ALLOC_START);
			for (usize j = 1; j < size_entries; ++j) {
				set_entry(i + j, BM_ALLOC_CONT);
			}
			auto* const ptr = index_to_ptr(i);
			// for sanity checking, and catching potential bugs
			if (!this->contains(ptr)) {
				panic("Allocated past the block, something went wrong");
			}
			return ptr;
		}
		return nullptr;
	}

	// Frees a pointer contained within this block, assuming it is here.
	void free_or_panic(void* ptr) {
		auto index = ptr_to_index(ptr);
		if (get_entry(index) != BM_ALLOC_START) {
			panic("Tried to heap free pointer that is not start of allocation! ({})", ptr);
		}
		set_entry(index, BM_FREE_SPACE);
		++index;
		for (; index < entries(); ++index) {
			if (get_entry(index) != BM_ALLOC_CONT) break;
			set_entry(index, BM_FREE_SPACE);
		}
	}

	void debug() const {
		kdbgln("HeapBlock[addr={}, next={}, size={:#x}]", this, next_block, block_size);
		kdbg(" -> Entries:");
		for (usize i = 0; i < entries(); ++i) {
			kdbg(" {:02b}", get_entry(i));
		}
		kdbgln("");
	}
};

HeapBlock* first_block = nullptr;

HeapBlock* allocate_block(usize size) {
	auto* block = reinterpret_cast<HeapBlock*>(kernel::alloc::allocate_pages(size / PAGE_SIZE));
	block->next_block = nullptr;
	block->block_size = size;
	memset(block->allocated, HeapBlock::BM_FREE_SPACE, block->entries_size());
	return block;
}

static constexpr usize BLOCK_ALLOC_SIZE = 16 * PAGE_SIZE;

void kernel::alloc::init_heap_allocator() {
	first_block = allocate_block(BLOCK_ALLOC_SIZE);
}

void* kernel::alloc::heap_allocate(usize size) {
	// Current design can't handle 0 size allocations (if they even have a purpose),
	// so return a nullptr
	if (size == 0) return nullptr;

	HeapBlock* prev_block = nullptr;
	for (auto* block = first_block; block != nullptr; block = block->next_block) {
		auto* ptr = block->try_allocate(size);
		if (ptr) return ptr;
		prev_block = block;
	}
	// try allocating a new block
	auto new_block_size = BLOCK_ALLOC_SIZE;
	if (size > new_block_size) {
		// TODO: maybe special block type for large allocations?
		new_block_size = HeapBlock::calculate_block_size_for_big_alloc(size);
	}
	kdbgln("Allocating a new heap block with size={:#x}..", new_block_size);
	prev_block->next_block = allocate_block(new_block_size);
	return prev_block->next_block->try_allocate(size);
}

void kernel::alloc::heap_free(void* ptr) {
	// do nothing if trying to free a nullptr
	if (ptr == nullptr) return;

	for (auto* block = first_block; block != nullptr; block = block->next_block) {
		if (block->contains(ptr)) {
			block->free_or_panic(ptr);
			return;
		}
	}
	panic("Tried to free invalid pointer ({})", ptr);
}

void* operator new(usize size) {
	return kernel::alloc::heap_allocate(size);
}

void operator delete(void* ptr) {
	kernel::alloc::heap_free(ptr);
}

void operator delete(void* ptr, usize) {
	operator delete(ptr);
}
