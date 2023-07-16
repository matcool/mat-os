#pragma once

#include <stl/types.hpp>

namespace kernel {

static constexpr usize PAGE_SIZE = 4096;

class VirtualAddress;

class PhysicalAddress {
	uptr m_value;
public:
	explicit PhysicalAddress(uptr value) : m_value(value) {}
	PhysicalAddress(VirtualAddress);

	VirtualAddress to_virtual() const;

	auto value() const { return m_value; }
};

class VirtualAddress {
	uptr m_value;
public:
	explicit VirtualAddress(uptr value) : m_value(value) {}
	VirtualAddress(PhysicalAddress);

	PhysicalAddress to_physical() const;

	auto value() const { return m_value; }
	void* ptr() const { return reinterpret_cast<void*>(m_value); }
};

namespace paging {

void init();

// Maps a physical address to virtual address, assuming limine's memory mapping
uptr physical_to_virtual(uptr physical_address);

// Maps a virtual address to physical address, assuming limine's memory mapping
uptr virtual_to_physical(uptr virtual_address);

}


}