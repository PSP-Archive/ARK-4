#include <pspsdk.h>
#include <ark.h>
#include <macros.h>
#include <cache.h>
#include "ansi_c_functions.h"

void patchMainBin(void)
{
    // For future patches (if we ever support cold boot of 1.50)

    //ClearCache();

    __asm(".set noreorder\n"
          "lui		$25, 0x0400\n"
          "lui		$sp, 0x040F\n"
          "jr		$25\n"
          "ori		$sp,$sp, 0xFF00\n"
          ".set reorder");
}

void _arkBoot() __attribute__ ((section (".text.startup")));
void _arkBoot()
{
    // Redirect jump to main.bin
    MAKE_JUMP(0x040f00ec, patchMainBin);

    // Reset exploit has unmapped the pre-ipl
    _sh(0xbfc0, 0x040f0030);

    ClearCache();

    void (* entry)() = (void *)0x040f0000;

    return entry();
}