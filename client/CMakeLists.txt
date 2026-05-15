cmake_minimum_required(VERSION 3.16)
project(client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# Add the myclib subdirectory (static library)
# -------------------------------------------------------------------
add_subdirectory(myclib)

# -------------------------------------------------------------------
# Client executable sources (application code)
# -------------------------------------------------------------------
set(APP_SOURCES
    src/main.c
    src/bot.c
    src/net.c
    src/protocol.c
)

add_executable(client ${APP_SOURCES})

# -------------------------------------------------------------------
# Link against myclib and use its include directories
# -------------------------------------------------------------------
target_link_libraries(client PRIVATE myclib)

# -------------------------------------------------------------------
# Compiler flags for the executable (must match freestanding)
# -------------------------------------------------------------------
target_compile_options(client PRIVATE
    -ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -fno-builtin
    -Wall -Wextra -Wpedantic
)

# -------------------------------------------------------------------
# Linker flags: do not use any system libc, produce static binary
# -------------------------------------------------------------------
target_link_options(client PRIVATE -nostdlib -static)

# -------------------------------------------------------------------
# Include directories for application headers (bot.h, net.h, protocol.h)
# -------------------------------------------------------------------
target_include_directories(client PRIVATE inc)