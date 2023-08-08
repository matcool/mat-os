#include <kernel/memory/paging.hpp>
#include <stl/format.hpp>
#include <stl/math.hpp>

using math::bit_mask;

namespace kernel::paging {

constexpr bool PageTableEntry::get_bit(u64 idx) const {
	return m_value & (1 << idx);
}

constexpr void PageTableEntry::set_bit(u64 idx, bool value) {
	m_value = math::set_bit(m_value, idx, value);
}

kernel::PhysicalAddress PageTableEntry::addr() const {
	static constexpr auto mask = ((u64(1) << 52) - 1) & ~((u64(1) << 12) - 1);
	return kernel::PhysicalAddress(m_value & mask);
}

void PageTableEntry::set_addr(PhysicalAddress addr) {
	const auto value = addr.value();
	m_value = (m_value & ~(bit_mask<u64>(48 - 12) << 12)) |
		(value & ~bit_mask<u64>(12) & bit_mask<u64>(48));
}

PageTableEntry* PageTableEntry::follow() const {
	return reinterpret_cast<PageTableEntry*>(this->addr().to_virtual().ptr());
}

bool PageTableEntry::is_present() const {
	return this->get_bit(0);
}

void PageTableEntry::set_present(bool value) {
	this->set_bit(0, value);
}

bool PageTableEntry::is_writable() const {
	return this->get_bit(1);
}

void PageTableEntry::set_writable(bool value) {
	this->set_bit(1, value);
}

bool PageTableEntry::is_user() const {
	return this->get_bit(2);
}

void PageTableEntry::set_user(bool value) {
	this->set_bit(2, value);
}

bool PageTableEntry::is_ps() const {
	return this->get_bit(7);
}

void PageTableEntry::set_ps(bool value) {
	this->set_bit(7, value);
}

bool PageTableEntry::is_execution_disabled() const {
	return this->get_bit(63);
}

void PageTableEntry::set_execution_disabled(bool value) {
	this->set_bit(63, value);
}

u16 PageTableEntry::get_available() const {
	// leave out last bit as it can be the Dirty bit for PT entries
	return (this->value() >> 47 & 0b1111111111100000) |
		(this->value() >> 7 & 0b11110); // | (value() >> 6 & 1);
}

void PageTableEntry::set_available(u16 value) {
	// set_bit(6, value & 1);
	m_value = (m_value & ~(bit_mask<u64>(4) << 8)) | ((u64(value) & 0b11110) << 7);
	m_value = (m_value & ~(bit_mask<u64>(11) << 52)) | (u64(value) >> 5 << 52);
}

void PageTableEntry::clear() {
	m_value = 0;
}

}