#include "paging.hpp"
#include "kernel/allocator.hpp"
#include "log.hpp"
#include "intrinsics.hpp"
#include <limine/limine.h>
#include <stl/format.hpp>
#include <stl/span.hpp>
#include <stl/math.hpp>
#include <stl/memory.hpp>

// limine maps physical memory -> virtual memory by just adding a higher half base
// this is constant except for when KASLR is on, so use this to get it
static volatile limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = nullptr,
};
uptr hhdm_base;

uptr kernel::paging::physical_to_virtual(uptr addr) {
	// limine identity maps everything up to 4 gib, need to deal with that later
	return addr + hhdm_base;
}

uptr kernel::paging::virtual_to_physical(uptr addr) {
	if (addr > hhdm_base)
		return addr - hhdm_base;
	return addr;
}

kernel::VirtualAddress::VirtualAddress(PhysicalAddress addr)
	: m_value(paging::physical_to_virtual(addr.value())) {}

kernel::PhysicalAddress::PhysicalAddress(VirtualAddress addr)
	: m_value(paging::virtual_to_physical(addr.value())) {}

kernel::PhysicalAddress kernel::VirtualAddress::to_physical() const {
	return PhysicalAddress(*this);
}

kernel::VirtualAddress kernel::PhysicalAddress::to_virtual() const {
	return VirtualAddress(*this);
}

kernel::PhysicalAddress kernel::PhysicalAddress::operator+(uptr offset) const {
	return PhysicalAddress(value() + offset);
}

template <class Func>
struct mat::Formatter<Func, kernel::paging::PageTableEntry> {
	static void format(Func func, kernel::paging::PageTableEntry entry) {
		mat::format_to(func, "[P={:d}, W={:d}, US={:d}, PS={:d}, avail={:04x}, addr={:#08x}], raw={:#x}",
			entry.is_present(), entry.is_writable(), entry.is_user(), entry.is_ps(), entry.get_available(), entry.addr().value(), entry.value());
	}
};

namespace kernel::paging {

constexpr bool PageTableEntry::get_bit(u64 idx) const {
	return m_value & (1 << idx);
}

constexpr void PageTableEntry::set_bit(u64 idx, bool value) {
	mat::math::set_bit(m_value, idx, value);
}

kernel::PhysicalAddress PageTableEntry::addr() const {
	static constexpr auto mask = ((u64(1) << 52) - 1) & ~((u64(1) << 12) - 1);
	return kernel::PhysicalAddress(m_value & mask);
}

void PageTableEntry::set_addr(PhysicalAddress addr) {
	const auto value = addr.value();
	m_value = (m_value & ~(mat::math::bit_mask<u64>(48 - 12) << 12)) | (value & ~mat::math::bit_mask<u64>(12) & mat::math::bit_mask<u64>(48));
}

PageTableEntry* PageTableEntry::follow() const {
	return reinterpret_cast<PageTableEntry*>(addr().to_virtual().ptr());
}

bool PageTableEntry::is_present() const {
	return get_bit(0);
}
void PageTableEntry::set_present(bool value) {
	set_bit(0, value);
}

bool PageTableEntry::is_writable() const {
	return get_bit(1);
}
void PageTableEntry::set_writable(bool value) {
	set_bit(1, value);
}

bool PageTableEntry::is_user() const {
	return get_bit(2);
}
void PageTableEntry::set_user(bool value) {
	set_bit(2, value);
}

bool PageTableEntry::is_ps() const {
	return get_bit(7);
}
void PageTableEntry::set_ps(bool value) {
	set_bit(7, value);
}

bool PageTableEntry::is_execution_disabled() const {
	return get_bit(63);
}
void PageTableEntry::set_execution_disabled(bool value) {
	set_bit(63, value);
}

u16 PageTableEntry::get_available() const {
	return (value() >> 47 & 0b1111111111100000) | (value() >> 7 & 0b11110) | (value() >> 6 & 1);
}

void PageTableEntry::set_available(u16 value) {
	// set_bit(6, value & 1);
	m_value = (m_value & ~(mat::math::bit_mask<u64>(4) << 8)) | ((u64(value) & 0b11110) << 7);
	m_value = (m_value & ~(mat::math::bit_mask<u64>(11) << 52)) | (u64(value) >> 5 << 52);
	// m_value = 0x69;
}

}

void kernel::paging::init() {
	if (!hhdm_request.response)
		halt();

	hhdm_base = hhdm_request.response->offset;

	kdbgln("CR0: {:#032b}", get_cr0());
	kdbgln("CR3: {:#032b}", get_cr3());
	kdbgln("CR4: {:#032b}", get_cr4());

	kdbgln("Paging initialized");
}

// Returns the initial paging table, here its PML4
kernel::paging::PageTableEntry* get_base_entries() {
	const auto entries_addr = kernel::PhysicalAddress(get_cr3() & ~u64(0b11111));
	return reinterpret_cast<kernel::paging::PageTableEntry*>(entries_addr.to_virtual().ptr());
}

void kernel::paging::explore_addr(uptr target_addr) {
	auto* entries = get_base_entries();
	
	kdbgln("Target addr ({:#x}) entries are:", target_addr);

	PhysicalAddress phys_addr(0);

	static constexpr auto mask9 = mat::math::bit_mask<u64>(9);

	auto& pml4 = entries[target_addr >> 39 & mask9];
	kdbgln("PML4 = {}", pml4);

	auto& pdpt = pml4.follow()[target_addr >> 30 & mask9];
	kdbgln("PDPT = {}", pdpt);
	
	if (pdpt.is_ps()) {
		// 1 GiB pages
		kdbgln("Stopping early, this is a 1 GiB page");
		auto phys_page = pdpt.addr();
		phys_addr = phys_page + (target_addr & mat::math::bit_mask<u64>(30));
	} else {
		auto& pd = pdpt.follow()[target_addr >> 21 & mask9];
		kdbgln("  PD = {}", pd);

		if (pd.is_ps()) {
			// 2 MiB pages
			kdbgln("Stopping early, this is a 2 MiB page");
			auto phys_page = pd.addr();
			phys_addr = phys_page + (target_addr & mat::math::bit_mask<u64>(21));
		} else {
			auto& pt = pd.follow()[target_addr >> 12 & mask9];
			kdbgln("  PT = {}", pt);

			// 4 KiB pages
			auto phys_page = pt.addr();
			phys_addr = phys_page + (target_addr & mat::math::bit_mask<u64>(12));
		}
	}

	kdbgln("Physical addr (from the page table)   is {:#x}", phys_addr.value());

	auto actual_phys = VirtualAddress(target_addr).to_physical().value();
	kdbgln("Physical addr (from subtracting HHDM) is {:#x}", actual_phys);

	auto virt = phys_addr.to_virtual();

	kdbgln("target_addr     is: {:#x}", target_addr);
	kdbgln("Physical + HHDM is: {:#x}", virt.value());

	auto* bytes = reinterpret_cast<u8*>(target_addr);
	kdbgln("Bytes at target_addr:      {:02x} {:02x} {:02x} {:02x}", bytes[0], bytes[1], bytes[2], bytes[3]);

	auto* bytes_virt = reinterpret_cast<u8*>(virt.ptr());
	
	kdbgln("Bytes at phys_addr + HHDM: {:02x} {:02x} {:02x} {:02x}", bytes_virt[0], bytes_virt[1], bytes_virt[2], bytes_virt[3]);
}

// if present, this means the table was allocated here, not by limine
static constexpr u16 MAT_TABLE_MAGIC = 0x4444;
// if present, the page is already mapped and should not try to map again
static constexpr u16 MAT_MAPPED_MAGIC = 0xf3f0;

// Currently will replace whatever was mapped there, as its probably limine's identity mapping
void kernel::paging::map_page(VirtualAddress virt, PhysicalAddress phys) {
	auto* entries = get_base_entries();

	static constexpr auto mask9 = mat::math::bit_mask<u64>(9);

	const auto index_pml4 = virt.value() >> 39 & mask9;
	const auto index_pdp = virt.value() >> 30 & mask9;
	const auto index_pd = virt.value() >> 21 & mask9;
	const auto index_pt = virt.value() >> 12 & mask9;

	// allocates a page table if its not present
	static constexpr auto allocate_entry_and_follow = [](PageTableEntry& entry) {
		// kdbgln("at entry {}", entry);
		// check if this is a big page too, in which case we create a new
		// table anyways, since we only care about 4 kib pages
		if (!entry.is_present() || entry.is_ps()) {
			// kdbgln("going in!");
			entry.set_available(MAT_TABLE_MAGIC);
			entry.set_present(true);
			entry.set_writable(true);
			entry.set_user(true);
			entry.set_ps(false);
			entry.set_execution_disabled(false);

			auto page = kernel::alloc::allocate_physical_page();
			mat::memset(page.to_virtual().ptr(), 0, PAGE_SIZE);
			
			entry.set_addr(page);
			// kdbgln("entry now is {}, page is {:#x}", entry, page.value());
		}
		return entry.follow();
	};

	auto& entry_pml4 = entries[index_pml4];
	auto& entry_pdp = allocate_entry_and_follow(entry_pml4)[index_pdp];
	auto& entry_pd = allocate_entry_and_follow(entry_pdp)[index_pd];
	auto& entry = allocate_entry_and_follow(entry_pd)[index_pt];

	if (entry.is_present() && entry.get_available() == MAT_MAPPED_MAGIC) {
		kdbgln("[PANIC] Tried to map to address that was already mapped! ({:#x} trying to {:#x}, but is {:#x})",
			virt.value(), phys.value(), entry.addr().value());
		halt();
	}

	entry.set_available(MAT_MAPPED_MAGIC);
	entry.set_present(true);
	entry.set_writable(true);
	entry.set_user(true);
	entry.set_execution_disabled(false);
	entry.set_addr(phys);

	// kdbgln("final entry is now {}", entry);
}