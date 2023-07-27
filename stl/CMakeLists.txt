add_library(stl INTERFACE)

target_include_directories(stl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_sources(stl INTERFACE
	format.cpp
	string.cpp
	memory.cpp
	random.cpp
	units.cpp
)