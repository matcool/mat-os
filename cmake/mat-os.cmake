set(COMMON_C_CXX_FLAGS "\
	-Wall -Wextra \
	-ffreestanding -fno-stack-protector -fno-stack-check \
	-fno-lto -fPIE \
	-m64 -march=x86-64 -mabi=sysv \
	-mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone \
	-O2 -g \
	-nostdlib")

add_compile_definitions(MAT_OS=1)
add_compile_definitions(USE_STL_NAMESPACE=1)
set(MAT_OS true)

set(CMAKE_C_FLAGS "${COMMON_C_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${COMMON_C_CXX_FLAGS} -fno-exceptions -fno-rtti -fmodules-ts")
