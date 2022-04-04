#pragma once
#include <lib/string.hpp>
#include <lib/vector.hpp>
#include <lib/array.hpp>
#include <lib/optional.hpp>

namespace kernel::filesystem {
	void init();

	struct File {
		String name;
		Vector<u8> data;
	};

	View<File> get_files();
	File* add_file(StringView name);
	// TODO make this a ref
	Optional<File*> get_file(StringView name);
}
