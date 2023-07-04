cmake_minimum_required(VERSION 3.20)

project(kernel)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(COMMON_C_CXX_FLAGS "-Wall -Wextra -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fPIE -m64 -march=x86-64 -mabi=sysv -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -g -O2 -nostdlib")

set(CMAKE_C_FLAGS ${COMMON_C_CXX_FLAGS})
set(CMAKE_CXX_FLAGS "${COMMON_C_CXX_FLAGS} -fno-exceptions -fno-rtti -fmodules-ts")
add_link_options("LINKER:-T,${CMAKE_CURRENT_SOURCE_DIR}/kernel/linker.ld")

add_executable(${PROJECT_NAME} kernel/serial.cpp kernel/kernel.cpp kernel/cxa.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	# DEPENDS build-iso.sh limine.cfg
	BYPRODUCTS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.iso
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build-iso.sh $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.iso ${CMAKE_BINARY_DIR}/iso_root
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	VERBATIM
)