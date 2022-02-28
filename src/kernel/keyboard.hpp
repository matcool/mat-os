#pragma once
#include "common.hpp"
#include "lib/function.hpp"
#include "lib/vector.hpp"

class KeyboardDispatcher {
	Vector<Function<void(u8)>> m_listeners;
public:
	[[nodiscard]] static KeyboardDispatcher& get();

	void add_listener(Function<void(u8)> callback);
	void dispatch(u8 key);
};

void keyboard_init();
