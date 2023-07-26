#include <kernel/log.hpp>
#include <stl/types.hpp>

extern "C" {
	// https://libcxxabi.llvm.org/spec.html

	int __cxa_guard_acquire(u64* guard) {
		// Effects: This function is called before initialization takes place.
		// Returns: 1 if the initialization is not yet complete, otherwise 0.
		return *reinterpret_cast<u8*>(guard) == 0;
	}

	void __cxa_guard_release(u64* guard) {
		// Effects: Sets the first byte of the guard object to a non-zero value.
		*reinterpret_cast<u8*>(guard) = 0x44;
	}

	// https://itanium-cxx-abi.github.io/cxx-abi/abi.html#dso-dtor-runtime-api

	void* __dso_handle;

	// this is for registering destructors on exit, however since the kernel never really "exits", its fine to not do anything
	int __cxa_atexit(void (*)(void*), void*, void*) {
		// > It returns zero if registration is successful, nonzero on failure.
		return 0;
	}
}