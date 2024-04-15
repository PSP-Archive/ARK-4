OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)

ENTRY(_start)

SECTIONS
{
	. = 0x40c0000;
	.init : { *(.init.start) *(.init) }
	.text.start : {
		*(.text.start)
	}
	.text : {
		*(.text)
	}
	.rodata : {
		*(.rodata)
	}
	.data : {
		*(.sdata)
		*(.data)
	}
	.bss : {
        __bss_start = .;
        *(.bss*)
        __bss_end = .;
    }
}
