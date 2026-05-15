cmake_minimum_required(VERSION 3.16)
project(client C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ------------------------------------------------------------
# 1. Locate pre‑built myclib
# ------------------------------------------------------------
set(MYCLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../myclib)
set(MYCLIB_INSTALL_DIR ${MYCLIB_ROOT}/build/myclib)

if(NOT EXISTS ${MYCLIB_INSTALL_DIR})
    message(FATAL_ERROR "myclib not built. Please run: cd ../myclib && mkdir build && cd build && cmake .. && make")
endif()

set(MYCLIB_INCLUDE ${MYCLIB_INSTALL_DIR}/include)
set(MYCLIB_LIBRARY ${MYCLIB_INSTALL_DIR}/lib/libmyclib.a)
set(MYCLIB_CRT1    ${MYCLIB_INSTALL_DIR}/lib/crt1.o)

# ------------------------------------------------------------
# 2. Compiler flags (freestanding, no system libc)
# ------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -fno-builtin")

# ------------------------------------------------------------
# 3. Build client sources as a static library
# ------------------------------------------------------------
set(CLIENT_SOURCES
    src/bot.c
    src/main.c
    src/net.c
    src/protocol.c
)

add_library(client_lib STATIC ${CLIENT_SOURCES})
target_include_directories(client_lib PRIVATE
    ${MYCLIB_INCLUDE}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
target_compile_options(client_lib PRIVATE -Wall -Wextra -Wpedantic)

# ------------------------------------------------------------
# 4. Extract object files from the static library (like previous ar x)
# ------------------------------------------------------------
set(EXTRACT_OBJ_DIR ${CMAKE_BINARY_DIR}/client_objs)
add_custom_command(
    TARGET client_lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTRACT_OBJ_DIR}
    COMMAND cd ${EXTRACT_OBJ_DIR} && ar x $<TARGET_FILE:client_lib>
    COMMENT "Extracting client object files"
)

# ------------------------------------------------------------
# 5. Provide the linker script (create if missing)
# ------------------------------------------------------------
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker/merge.ld)
if(NOT EXISTS ${LINKER_SCRIPT})
    file(WRITE ${LINKER_SCRIPT} "
SECTIONS
{
    . = 0;
    .text : {
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
    }
    .data : {
        *(.data)
        *(.bss)
    }
    /DISCARD/ : {
        *(.note*)
        *(.comment)
        *(.eh_frame)
        *(.debug*)
    }
}
")
    message(STATUS "Created default linker script: ${LINKER_SCRIPT}")
endif()

# ------------------------------------------------------------
# 6. Full static link to produce a resolved ELF (combined.elf)
# ------------------------------------------------------------
set(COMBINED_ELF ${CMAKE_BINARY_DIR}/combined.elf)

add_custom_command(
    OUTPUT ${COMBINED_ELF}
    DEPENDS client_lib ${MYCLIB_CRT1} ${MYCLIB_LIBRARY}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTRACT_OBJ_DIR}
    COMMAND ${CMAKE_LINKER} -static -T ${LINKER_SCRIPT} -o ${COMBINED_ELF}
        ${MYCLIB_CRT1}
        ${EXTRACT_OBJ_DIR}/*.o
        ${MYCLIB_LIBRARY}
    COMMENT "Linking fully resolved ELF: combined.elf"
)
add_custom_target(combined_elf ALL DEPENDS ${COMBINED_ELF})

# ------------------------------------------------------------
# 7. (Optional) Also produce a standalone test executable
# ------------------------------------------------------------
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