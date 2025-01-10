#include <kernel/memory/paging.hpp>

kernel::VirtualAddress::VirtualAddress(PhysicalAddress addr) :
	m_value(paging::physical_to_virtual(addr.value())) {}

kernel::PhysicalAddress::PhysicalAddress(VirtualAddress addr) :
	m_value(paging::virtual_to_physical(addr.value())) {}

kernel::PhysicalAddress kernel::VirtualAddress::to_physical() const {
	return PhysicalAddress(*this);
}

kernel::VirtualAddress kernel::PhysicalAddress::to_virtual() const {
	return VirtualAddress(*this);
}

kernel::PhysicalAddress kernel::PhysicalAddress::operator+(uptr offset) const {
	return PhysicalAddress(value() + offset);
}

kernel::VirtualAddress kernel::VirtualAddress::operator+(uptr offset) const {
	return VirtualAddress(value() + offset);
}

kernel::VirtualAddress kernel::VirtualAddress::to_hhdm() const {
	return this->to_physical().to_virtual();
}