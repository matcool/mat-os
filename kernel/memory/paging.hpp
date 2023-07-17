#pragma once

#include <stl/types.hpp>

namespace kernel {

static constexpr usize PAGE_SIZE = 4096;

class VirtualAddress;

class PhysicalAddress {
	uptr m_value;
public:
	explicit PhysicalAddress(uptr value) : m_value(value) {}
	PhysicalAddress() : PhysicalAddress(0) {}
	PhysicalAddress(VirtualAddress);

	// Converts to a HHDM virtual address
	VirtualAddress to_virtual() const;

	auto value() const { return m_value; }

	PhysicalAddress operator+(uptr offset) const;
};

class VirtualAddress {
	uptr m_value;
public:
	explicit VirtualAddress(uptr value) : m_value(value) {}
	VirtualAddress() : VirtualAddress(0) {}
	VirtualAddress(PhysicalAddress);

	PhysicalAddress to_physical() const;

	auto value() const { return m_value; }
	void* ptr() const { return reinterpret_cast<void*>(m_value); }

	VirtualAddress operator+(uptr offset) const;
};

namespace paging {

class PageTableEntry {
	u64 m_value;

	constexpr bool get_bit(u64 idx) const;
	constexpr void set_bit(u64 idx, bool value);
public:
	PageTableEntry(u64 value) : m_value(value) {}

	auto value() const { return m_value; }
	auto& value() { return m_value; }

	PhysicalAddress addr() const;
	void set_addr(PhysicalAddress addr);

	PageTableEntry* follow() const;

	// P flag, must be true if the entry should be used.
	bool is_present() const;
	void set_present(bool value);

	// R/W flag, if true then the page is writable.
	bool is_writable() const;
	void set_writable(bool value);

	// U/S flag, if true then this page is accessible to userspace apps.
	bool is_user() const;
	void set_user(bool value);

	// PS flag, if true then this entry points to a page larger than 4 KiB,
	// either 2 MiB or 1 GiB. If this is a PT entry then this is not PS, but PAT
	bool is_ps() const;
	void set_ps(bool value);

	bool is_execution_disabled() const;
	void set_execution_disabled(bool value);

	// get available bits in the entry, which the cpu ignores.
	// if this is an entry that points to a page, then some bits may be used
	// if PGE or PKS are enabled.
	u16 get_available() const;
	void set_available(u16 value);

	// clears the entry, setting it to 0
	void clear();
};

void init();

// Maps a physical address to virtual address, assuming limine's memory mapping
uptr physical_to_virtual(uptr physical_address);

// Maps a virtual address to physical address, assuming limine's memory mapping
uptr virtual_to_physical(uptr virtual_address);

void explore_addr(uptr value);

// Maps a physical page to a virtual address.
// TODO: add flags, and maybe page size
void map_page(VirtualAddress virt, PhysicalAddress phys);

// Unmaps a page, making it not present.
void unmap_page(VirtualAddress virt);

// Invalidates the TLB cache for a certain page.
void invalidate_cache(VirtualAddress virt);

}


}