#pragma once

#include <stl/string.hpp>
#include <stl/vector.hpp>

namespace kernel {

// This class stores info about all kernel modules, which are specified in the limine config.
class Modules {
public:
	struct Module {
		StringView path;
		const void* addr;
		usize size;
	};

private:
	Vector<Module> m_modules;

	Modules();

public:
	static Modules& get();

	// Gets a module by path.
	const Module& with_path(StringView path) const;
};

}