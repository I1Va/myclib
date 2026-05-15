cmake_minimum_required(VERSION 3.16)
project(combined_elf C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(myclib)
add_subdirectory(clientlib)
add_subdirectory(wrapperlib)

set(MYCLIB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/myclib)
set(MYCLIB_LIBRARY    ${MYCLIB_OUTPUT_DIR}/lib/libmyclib.a)
set(CLIENTLIB_LIBRARY $<TARGET_FILE:clientlib>)
set(WRAPPERLIB_LIBRARY ${CMAKE_BINARY_DIR}/wrapperlib/lib/libwrapperlib.a)

set(DEPENDS_LIST myclib clientlib wrapperlib)
set(LIBS_LIST 
${MYCLIB_LIBRARY} 
${CLIENTLIB_LIBRARY} 
${WRAPPERLIB_LIBRARY}
)


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

set(COMBINED_ELF ${CMAKE_BINARY_DIR}/combined.elf)

add_custom_command(
    OUTPUT ${COMBINED_ELF}
    DEPENDS ${DEPENDS_LIST} 
    COMMAND ${CMAKE_C_COMPILER} -nostdlib -static -T ${LINKER_SCRIPT}
        -Wl,--whole-archive ${LIBS_LIST} 
        -Wl,--no-whole-archive
        -o ${COMBINED_ELF}
    COMMENT "Linking combined.elf"
)

add_custom_target(combined_elf ALL DEPENDS ${COMBINED_ELF})