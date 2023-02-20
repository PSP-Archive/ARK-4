OUTPUT_FORMAT("elf32-littlemips", "elf32-bigmips", "elf32-littlemips")
OUTPUT_ARCH(mips:allegrex)

ENTRY(_start)

SECTIONS
{
    . = 0x88FC0000;
    .text :
    {
        KEEP(*(.text.vector))
        *(.text)
    }
    .sdata : { *(.sdata) *(.sdata.*) *(.gnu.linkonce.s.*) }
    .rodata : { *(.rodata) }
    .data : { *(.data*) }
    . = .;
    .bss (NOLOAD):
    {
        __bss_start = .;
        *(.bss* .bss.*)
        *(COMMON)
        __bss_end = .;
    }
}
