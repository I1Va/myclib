cmake_minimum_required(VERSION 3.16)
project(myclib VERSION 1.0.0 LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# Freestanding flags for C (no libc, no stack protector, no red zone)
# -------------------------------------------------------------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fno-stack-protector -mno-red-zone -fno-builtin")

# Assembly flags – keep simple (no -nostdlib)
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS}")

# -------------------------------------------------------------------
# 1. Static library (all C sources)
# -------------------------------------------------------------------
set(LIB_SOURCES
    src/mystring.c
    src/mystdio.c
    src/mystdlib.c
    src/myunistd.c
    src/myerrno.c
)
add_library(myclib STATIC ${LIB_SOURCES})

target_include_directories(myclib PRIVATE inc)

# -------------------------------------------------------------------
# 2. Startup object (crt1.o) from assembly
# -------------------------------------------------------------------
set(CRT1_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/startup.s)
set(CRT1_OBJ ${CMAKE_CURRENT_BINARY_DIR}/crt1.o)

add_custom_command(
    OUTPUT ${CRT1_OBJ}
    COMMAND ${CMAKE_ASM_COMPILER} ${CMAKE_ASM_FLAGS} -c ${CRT1_SRC} -o ${CRT1_OBJ}
    DEPENDS ${CRT1_SRC}
    COMMENT "Assembling startup.s -> crt1.o"
)
add_custom_target(myclib_crt1 ALL DEPENDS ${CRT1_OBJ})


# -------------------------------------------------------------------
# 3. Copy headers to build directory (as "include/")
# -------------------------------------------------------------------
set(HEADER_DEST ${CMAKE_CURRENT_BINARY_DIR}/include)
add_custom_command(
    TARGET myclib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${HEADER_DEST}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/inc ${HEADER_DEST}
    COMMENT "Copying headers to build/include"
)