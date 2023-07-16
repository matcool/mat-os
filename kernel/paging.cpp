#include "paging.hpp"
#include "log.hpp"
#include "intrinsics.hpp"
#include <limine/limine.h>

// limine maps physical memory -> virtual memory by just adding a higher half base
// this is constant except for when KASLR is on, so use this to get it
static volatile limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = nullptr,
};
uptr hhdm_base;

uptr kernel::paging::physical_to_virtual(uptr addr) {
	return addr + hhdm_base;
}

uptr kernel::paging::virtual_to_physical(uptr addr) {
	return addr - hhdm_base;
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

void kernel::paging::init() {
	if (!hhdm_request.response)
		halt();

	hhdm_base = hhdm_request.response->offset;

	kdbgln("CR0: {:#032b}", get_cr0());
	kdbgln("CR3: {:#032b}", get_cr3());
	kdbgln("CR4: {:#032b}", get_cr4());

	kdbgln("Paging initialized");
}