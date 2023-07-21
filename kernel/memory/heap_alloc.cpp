#include <stl/memory.hpp>
#include <stl/math.hpp>
#include <kernel/memory/allocator.hpp>
#include <kernel/log.hpp>

using mat::math::div_ceil;

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

	static constexpr u8 BM_FREE_SPACE = 0b00;
	static constexpr u8 BM_ALLOC_START = 0b01;
	static constexpr u8 BM_ALLOC_CONT = 0b11;

	// amount of entries in the bitmap
	usize entries() const {
		return block_size / ALLOC_ALIGN;
	}

	// amount of bytes occupied by the bitmap array
	usize entries_size() const {
		// each entry takes up 2 bits, so (entries * 2) / 8
		return div_ceil<usize>(entries(), 8 / 2);
	}

	// total amount of bytes occupied by the metadata itself
	usize metadata_size() const {
		// round up to alloc_align so that the allocations are.. aligned
		return div_ceil(sizeof(*this) + entries_size(), ALLOC_ALIGN) * ALLOC_ALIGN;
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
		static constexpr auto entries_per_byte = 8 / 2;
		const auto offset = (index % entries_per_byte) * 2;
		return allocated[index / entries_per_byte] >> offset & 0b11;
	}

	void set_entry(usize index, u8 value) {
		static constexpr auto entries_per_byte = 8 / 2;
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
			return index_to_ptr(i);
		}
		return nullptr;
	}

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

void kernel::alloc::init_heap_allocator() {
	auto* block = allocate_pages(1);
	first_block = reinterpret_cast<decltype(first_block)>(block);
	first_block->next_block = nullptr;
	first_block->block_size = 1 * PAGE_SIZE;
	mat::memset(first_block->allocated, HeapBlock::BM_FREE_SPACE, first_block->entries_size());

	first_block->debug();
	auto* p1 = first_block->try_allocate(90);
	first_block->debug();
	auto* p2 = first_block->try_allocate(16);
	auto* p3 = first_block->try_allocate(16);
	auto* p4 = first_block->try_allocate(17);
	auto* p5 = first_block->try_allocate(16);
	first_block->debug();
	first_block->free_or_panic(p4);
	first_block->debug();
	first_block->try_allocate(33);
	first_block->debug();
	kdbgln("p1 = {}, p2 = {}", p1, p2);
}

void* kernel::alloc::heap_allocate(usize bytes) {
	return nullptr;
}

void kernel::alloc::heap_free(void* ptr) {

}