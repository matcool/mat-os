add_link_options("LINKER:-T,${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")

add_executable(kernel
	serial.cpp
	kernel.cpp
	cxa.cpp
	idt.cpp
	memory/physical_alloc.cpp
	memory/paging.cpp
	memory/virtual_alloc.cpp
	memory/address.cpp
	memory/page_entry.cpp
	memory/heap_alloc.cpp
	device/pic.cpp
	device/ps2.cpp
	device/keyboard.cpp
	device/pit.cpp
	device/mouse.cpp
	screen/framebuffer.cpp
	screen/canvas.cpp
	window/manager.cpp
	window/context.cpp
	window/widget.cpp
	window/window.cpp
	window/button.cpp
	terminal/window.cpp
	terminal/terminal.cpp
	tasks/scheduler.cpp
)

target_link_libraries(kernel limine stl)
# so i can include things as #include <kernel/memory/whatever.hpp>
target_include_directories(kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# thanks https://stackoverflow.com/a/76701360/9124836

set(ISO_PATH "${CMAKE_BINARY_DIR}/kernel.iso")
set(ISO_ROOT_PATH "${CMAKE_BINARY_DIR}/iso_root")
set(ISO_BUILD_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../build_iso.sh")

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