cmake_minimum_required(VERSION 3.16)
project(myclib VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Freestanding flags – no libc, no system headers, no stack protector
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -fno-builtin")

# Static library (all C sources)
set(LIB_SOURCES
    src/mycmp.c
    src/mystring.c
    src/mystdio.c
    src/mystdlib.c
    src/myunistd.c
    src/myerrno.c
)
add_library(myclib STATIC ${LIB_SOURCES})

target_include_directories(myclib PRIVATE inc)

# Output directory: ${CMAKE_BINARY_DIR}/myclib/
# Contains:
#   include/  (headers)
#   lib/
#       libmyclib.a
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/myclib)
set(OUTPUT_INCLUDE ${OUTPUT_DIR}/include)
set(OUTPUT_LIB ${OUTPUT_DIR}/lib)

# Copy headers
add_custom_command(
    TARGET myclib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_INCLUDE}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/inc ${OUTPUT_INCLUDE}
    COMMENT "Copying headers to ${OUTPUT_INCLUDE}"
)

# Copy libmyclib.a
add_custom_command(
    TARGET myclib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:myclib> ${OUTPUT_LIB}
    COMMENT "Copying libmyclib.a to ${OUTPUT_LIB}"
)
