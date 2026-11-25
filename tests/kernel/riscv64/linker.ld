ENTRY(_start)

SECTIONS
{
    . = 0x80000000;

    .text :
    {
        KEEP(*(.text.boot))
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
        *(.srodata*)
    }

    .data :
    {
        *(.data*)
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.sdata*)
    }

    .bss (NOLOAD) :
    {
        . = ALIGN(8);
        __bss_start = .;
        *(.sbss*)
        *(COMMON)
        *(.bss*)
        . = ALIGN(8);
        __bss_end = .;
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
        *(.riscv.attributes)
    }
}
