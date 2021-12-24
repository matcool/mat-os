extern "C" {
	void* __dso_handle;

	// TODO: do this properly Lol
	int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso) {

		return 0;
	}
	using guard = int;

	int __cxa_guard_acquire(guard*);
	void __cxa_guard_release(guard*);
	void __cxa_guard_abort(guard*);

	int __cxa_guard_acquire(guard* g) {
		return !*(char*)(g);
	}

	void __cxa_guard_release(guard* g) {
		*(char*)g = 1;
	}

	void __cxa_guard_abort(guard*) {}
}
