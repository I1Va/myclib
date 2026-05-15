cmake_minimum_required(VERSION 3.16)
project(client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------
# Freestanding flags (same as myclib)
# ------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -fno-builtin")

# ------------------------------------------------------------------
# Locate myclib headers (relative path, works when building from root)
# ------------------------------------------------------------------
set(MYCLIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../myclib/inc)

# ------------------------------------------------------------------
# Build static library
# ------------------------------------------------------------------
set(CLIENT_SOURCES
    src/bot.c
    src/main.c
    src/net.c
    src/protocol.c
)

add_library(clientlib STATIC ${CLIENT_SOURCES})

target_include_directories(clientlib PRIVATE
    ${MYCLIB_INCLUDE}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_compile_options(clientlib PRIVATE -Wall -Wextra -Wpedantic)

# ------------------------------------------------------------------
# Optional standalone test executable (links with myclib and uses crt1.o)
# This is for quick testing – the final combined.elf is built in root.
# ------------------------------------------------------------------
if(CLIENT_BUILD_TEST_EXE)
    # Locate myclib artifacts (assumes myclib is already built)
    set(MYCLIB_LIB_DIR ${CMAKE_BINARY_DIR}/myclib/lib)
    set(MYCLIB_CRT1    ${MYCLIB_LIB_DIR}/crt1.o)
    set(MYCLIB_LIBRARY ${MYCLIB_LIB_DIR}/libmyclib.a)

    add_executable(client_exe ${CLIENT_SOURCES})
    target_include_directories(client_exe PRIVATE
        ${MYCLIB_INCLUDE}
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
    )
    target_link_libraries(client_exe
        ${MYCLIB_CRT1}
        ${MYCLIB_LIBRARY}
    )
    target_link_options(client_exe PRIVATE -nostdlib -static)
endif()