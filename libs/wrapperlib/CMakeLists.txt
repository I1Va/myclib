cmake_minimum_required(VERSION 3.16)
project(wrapperlib C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Freestanding flags – no libc, no stack protector, no red zone
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -fno-builtin")
# For the assembler, we only need basic flags (e.g., -g for debug if desired)
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS}")

# ------------------------------------------------------------------
# Build the static library from the assembly source(s)
# ------------------------------------------------------------------
set(WRAPPER_SOURCES
    src/wrapper.s
)

add_library(wrapperlib STATIC ${WRAPPER_SOURCES})

# Optional: If wrapperlib needs to export include files (e.g., for function prototypes)
# Copy headers to output directory.
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/wrapperlib)
set(OUTPUT_INCLUDE ${OUTPUT_DIR}/include)
set(OUTPUT_LIB ${OUTPUT_DIR}/lib)

add_custom_command(
    TARGET wrapperlib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_INCLUDE}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/inc ${OUTPUT_INCLUDE}
    COMMENT "Copying wrapperlib headers to ${OUTPUT_INCLUDE}"
)

add_custom_command(
    TARGET wrapperlib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wrapperlib> ${OUTPUT_LIB}
    COMMENT "Copying wrapperlib.a to ${OUTPUT_LIB}"
)

# If you want to also copy the assembly source or other artifacts, add similar commands.