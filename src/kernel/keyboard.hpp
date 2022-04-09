#pragma once
#include "common.hpp"
#include "lib/function.hpp"
#include "lib/vector.hpp"

enum class Key : u32 {
	BACKSPACE = 8,
	ENTER = '\n',

	UP_ARROW = 256,
	DOWN_ARROW = 257,
	LEFT_ARROW = 258,
	RIGHT_ARROW = 259
};

class KeyboardDispatcher {
	Vector<Function<void(Key)>> m_listeners;
public:
	[[nodiscard]] static KeyboardDispatcher& get();

	void add_listener(Function<void(Key)> callback);
	void dispatch(Key key);
};

void keyboard_init();
