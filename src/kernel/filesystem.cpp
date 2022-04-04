#include "filesystem.hpp"

namespace kernel::filesystem {

	static Vector<File> files;

	void init() {

	}

	View<File> get_files() {
		return {files.data(), files.size()};
	}

	File* add_file(StringView name) {
		files.push_back(File { name, {} });
		return &files.back();
	}

	Optional<File*> get_file(StringView name) {
		for (auto& file : files) {
			if (file.name == name) return &file;
		}
		return {};
	}
}
