
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
