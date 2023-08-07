#pragma once

#include <stl/format.hpp>
#include <stl/string.hpp>

namespace kernel::terminal {

struct TerminalClient {
	virtual void put(char ch) = 0;
};

class Terminal {
	TerminalClient* m_client = nullptr;

	void show_prompt();

	template <class... Args>
	void fmt(StringView str, const Args&... args) {
		format_to([this](char ch) { m_client->put(ch); }, str, args...);
	}

public:
	Terminal(TerminalClient*);

	void handle_input(StringView str);

	void init();
};

}