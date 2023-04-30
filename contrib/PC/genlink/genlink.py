#!/usr/bin/env python3

import sys
from string import Template

def main():
    if len(sys.argv) < 2:
        print ("Usage: %s <link addr> [entry function name]" % (sys.argv[0]))
        sys.exit(1)
    
    linkAddr = int(sys.argv[1], 16)
    linkfile(linkAddr)

def linkfile(linkAddr):
    t = '''\
OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)

ENTRY($entry)

SECTIONS
{
  . = $linkaddr;
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
}'''

    if len(sys.argv) < 3:
        entry = 'main'
    else:
        entry = sys.argv[2]

    print(Template(t).substitute(linkaddr = '0x%08X' % linkAddr, entry = entry))

if __name__ == "__main__":
    main()
