add_link_options("LINKER:-T,${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

add_executable(kernel
	serial.cpp
	kernel.cpp
	cxa.cpp
	idt.cpp
	allocator.cpp
	paging.cpp
	virtual_allocator.cpp
)

target_link_libraries(kernel limine stl)

# thanks https://stackoverflow.com/a/76701360/9124836

set(ISO_PATH "${CMAKE_CURRENT_BINARY_DIR}/kernel.iso")
set(ISO_ROOT_PATH "${CMAKE_CURRENT_BINARY_DIR}/iso-root")
set(ISO_BUILD_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../build-iso.sh")

add_custom_command(
	OUTPUT "${ISO_PATH}"
	COMMAND "${ISO_BUILD_SCRIPT}" "$<TARGET_FILE:kernel>" "${ISO_PATH}" "${ISO_ROOT_PATH}"
	DEPENDS
		"${ISO_BUILD_SCRIPT}"
		${CMAKE_CURRENT_SOURCE_DIR}/../limine.cfg
		kernel
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../"
	VERBATIM
)

add_custom_target(kernel-iso ALL DEPENDS "${ISO_PATH}")