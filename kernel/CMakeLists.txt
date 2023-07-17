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

add_custom_command(
	TARGET kernel POST_BUILD
	# don't seem to work :(
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../build-iso.sh ${CMAKE_CURRENT_SOURCE_DIR}/../limine.cfg
	BYPRODUCTS ${CMAKE_BINARY_DIR}/kernel.iso
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../build-iso.sh $<TARGET_FILE:kernel> ${CMAKE_BINARY_DIR}/kernel.iso ${CMAKE_BINARY_DIR}/iso_root
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../
	VERBATIM
)