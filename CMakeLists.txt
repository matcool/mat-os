cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

include(cmake/mat-os.cmake)

project(mat-os)

add_library(limine INTERFACE)
target_include_directories(limine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(stl)
add_subdirectory(kernel)