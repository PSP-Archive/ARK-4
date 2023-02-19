OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)

ENTRY(_arkBoot)

SECTIONS
{
  . = 0x040d6000;
  .text.startup : {
    *(.text.startup)
  }
  .text : {
    *(.text)
  }
  .rodata : {
    *(.rodata)
  }
  .data : {
    *(.data)
  }
  __bss_start = .;
  .bss : {
    *(.bss)
  }
  __bss_end = .;
}
