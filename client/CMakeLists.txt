cmake_minimum_required(VERSION 3.16)
project(client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# Path to myclib (relative to this file)
# -------------------------------------------------------------------
set(MYCLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/myclib)

# -------------------------------------------------------------------
# Freestanding compilation flags – no libc, no stack protector, no red zone
# -------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -fno-builtin")

# -------------------------------------------------------------------
# Include directories – myclib first to override any system headers
# -------------------------------------------------------------------
set(INC_DIRS
    ${MYCLIB_DIR}/include            # myclib headers (errno.h, stdio.h, …)
    ${CMAKE_CURRENT_SOURCE_DIR}/inc  # application headers (bot.h, net.h, protocol.h)
)

# -------------------------------------------------------------------
# Client source files
# -------------------------------------------------------------------
set(SOURCES
    src/main.c
    src/bot.c
    src/net.c
    src/protocol.c
)

add_executable(client ${SOURCES})

# -------------------------------------------------------------------
# Add include directories
# -------------------------------------------------------------------
target_include_directories(client PRIVATE ${INC_DIRS})

# -------------------------------------------------------------------
# Link with myclib: first the startup object (crt1.o), then the static library
# -------------------------------------------------------------------
target_link_libraries(client
    ${MYCLIB_DIR}/lib/crt1.o
    ${MYCLIB_DIR}/lib/libmyclib.a
)

# -------------------------------------------------------------------
# Linker flags – static, no default libraries
# -------------------------------------------------------------------
target_link_options(client PRIVATE -nostdlib -static)