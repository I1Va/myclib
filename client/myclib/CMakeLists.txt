cmake_minimum_required(VERSION 3.16)
project(myclib C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# Freestanding flags – no stack protector, no red zone, no libc
# -------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -fno-builtin")

# -------------------------------------------------------------------
# Static library sources (mini‑libc + startup)
# -------------------------------------------------------------------
set(LIB_SOURCES
    src/startup.s          # _start entry point
    src/string.c
    src/stdio.c
    src/stdlib.c
    src/unistd.c
    src/errno.c
)

add_library(myclib STATIC ${LIB_SOURCES})

# -------------------------------------------------------------------
# Public include directory for the library
# -------------------------------------------------------------------
target_include_directories(myclib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# -------------------------------------------------------------------
# (Optional) Disable warnings that are irrelevant for freestanding code
# -------------------------------------------------------------------
target_compile_options(myclib PRIVATE -Wall -Wextra -Wpedantic)