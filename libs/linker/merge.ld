
SECTIONS
{
    . = 0;
    .text : {
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(8);
        *(.data)
        *(.data.*)
        . = ALIGN(8);
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    /DISCARD/ : {
        *(.note*)
        *(.comment)
        *(.eh_frame)
        *(.debug*)
    }
}
