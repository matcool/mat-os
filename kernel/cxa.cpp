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
}