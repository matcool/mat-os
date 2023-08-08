#include <kernel/log.hpp>
#include <kernel/modules.hpp>
#include <limine/limine.h>

static volatile limine_module_request module_request = {
	.id = LIMINE_MODULE_REQUEST,
	.revision = 0,
	.response = nullptr,
};

namespace kernel {

Modules& Modules::get() {
	static Modules inst;
	return inst;
}

Modules::Modules() {
	if (!module_request.response) {
		panic("No response for kernel modules");
	}

	const auto modules =
		Span(module_request.response->modules, module_request.response->module_count);

	m_modules = modules.iter()
					.map([](limine_file* file) {
						return Modules::Module{
							.path = file->path,
							.addr = file->address,
							.size = file->size,
						};
					})
					.collect_vec();
}

const Modules::Module& Modules::with_path(StringView path) const {
	// TODO: HashMap :-)
	for (const auto& mod : m_modules) {
		if (mod.path == path) return mod;
	}
	panic("Failed to find module with path {}", path);
}

}