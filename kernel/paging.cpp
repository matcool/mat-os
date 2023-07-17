#include "paging.hpp"
#include "log.hpp"
#include "intrinsics.hpp"
#include <limine/limine.h>
#include <stl/format.hpp>
#include <stl/span.hpp>
#include <stl/math.hpp>

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

class PageTableEntry {
	u64 m_value;

	constexpr bool get_bit(u64 idx) const { return m_value & (1 << idx); }
	constexpr void set_bit(u64 idx, bool value) { mat::math::set_bit(m_value, idx, value); }
public:
	PageTableEntry(u64 value) : m_value(value) {}

	auto value() const { return m_value; }
	auto& value() { return m_value; }

	auto addr() const {
		static constexpr auto mask = ((u64(1) << 52) - 1) & ~((u64(1) << 12) - 1);
		return kernel::PhysicalAddress(m_value & mask);
	}

	auto* follow() const {
		return reinterpret_cast<PageTableEntry*>(addr().to_virtual().ptr());
	}

	// P flag, must be true if the entry should be used.
	bool is_present() const { return get_bit(0); }
	void set_present(bool value) { set_bit(0, value); }

	// R/W flag, if true then the page is writable.
	bool is_writable() const { return get_bit(1); }
	void set_writable(bool value) { set_bit(1, value); }

	// U/S flag, if true then this page is accessible to userspace apps.
	bool is_user() const { return get_bit(2); }
	void set_user(bool value) { set_bit(2, value); }

	// PS flag, if true then this entry points to a page larger than 4 KiB,
	// either 2 MiB or 1 GiB. If this is a PT entry then this is not PS, but PAT
	bool is_ps() const { return get_bit(7); }
	void set_ps(bool value) { set_bit(7, value); }

	bool is_execution_disabled() const { return get_bit(63); }
	void set_execution_disabled(bool value) { set_bit(63, value); }

	// get available bits in the entry, which the cpu ignores.
	// if this is an entry that points to a page, then some bits may be used
	// if PGE or PKS are enabled.
	u16 get_available() const {
		//      11 bits                4 bits                1 bit
		return (value() >> 52 << 5) | (value() >> 8 << 1) | (value() >> 6);
	}
	void set_available(u16 value) {
		set_bit(6, value & 1);
		m_value = (m_value & ~(mat::math::bit_mask<u64>(4) << 8)) | (value & 0b11110);
		m_value = (m_value & ~(mat::math::bit_mask<u64>(11) << 52)) | (value >> 5);
	}
};

template <class Func>
struct mat::Formatter<Func, PageTableEntry> {
	static void format(Func func, PageTableEntry entry) {
		mat::format_to(func, "[P={:d}, W={:d}, US={:d}, PS={:d}, addr={:#08x}], raw={:#x}",
			entry.is_present(), entry.is_writable(), entry.is_user(), entry.is_ps(), entry.addr().value(), entry.value());
	}
};

void kernel::paging::init() {
	if (!hhdm_request.response)
		halt();

	hhdm_base = hhdm_request.response->offset;

	kdbgln("CR0: {:#032b}", get_cr0());
	kdbgln("CR3: {:#032b}", get_cr3());
	kdbgln("CR4: {:#032b}", get_cr4());

	kdbgln("Paging initialized");
}

void kernel::paging::explore_addr(uptr target_addr) {
	auto entries_addr = PhysicalAddress(get_cr3() & ~u64(0b11111));
	auto entries = mat::Span(reinterpret_cast<PageTableEntry*>(entries_addr.to_virtual().ptr()), 512);
	
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