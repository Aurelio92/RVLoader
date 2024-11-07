OUTPUT_FORMAT("elf32-powerpc")
OUTPUT_ARCH(powerpc:common)

ENTRY(_start)

__base_addr = 0x80003400;

SECTIONS
{
    . = __base_addr;

    .init :
    {
        *(.init)
        . = ALIGN(4);
    }

    .text :
    {
        *(.text.*)
        *(.gnu.warning)
        *(.gnu.linkonce.t*)
        *(.glue_7)
        *(.glue_7t)
        . = ALIGN(4);
    }

    __text_end = . ;

    .rodata :
    {
        *(.rodata)
        *all.rodata*(*)
        *(.roda)
        *(.rodata.*)
        *(.gnu.linkonce.r*)
        . = ALIGN(4);
    }

    .data :
    {
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)
        . = ALIGN(4);
    }

    .bss :
    {
        __bss_start = . ;
        *(.dynbss)
        *(.gnu.linkonce.b*)
        *(.bss*)
        *(.sbss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = . ;
    }

    .stack :
    {
        . = ALIGN(4);
    }
}

PROVIDE (__bss_start = __bss_start);
PROVIDE (__bss_end = __bss_end);
