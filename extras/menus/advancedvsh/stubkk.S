	.set noreorder

#include "pspstub.s"

STUB_START "scePower", 0x40010000, 0x00030005
STUB_FUNC 0x2B7C7CF4, scePowerRequestStandby
STUB_FUNC 0x0442D852, scePowerRequestColdReset
STUB_FUNC 0xAC32C9CC, scePowerRequestSuspend
STUB_END

STUB_START "scePaf",0x00090011,0x00120005
STUB_FUNC  0xAF067FA2,scePaf_wcslen
STUB_FUNC  0xA138A376,scePaf_sprintf
STUB_FUNC  0xB4652CFE,scePaf_memcpy
STUB_FUNC  0xB05D9677,scePaf_memcmp
STUB_FUNC  0xD9E2D6E1,scePaf_memset
STUB_FUNC  0xD7DCB972,scePaf_strlen
STUB_FUNC  0x726DFBA9,scePaf_strcpy
STUB_FUNC  0x706ABBFF,scePaf_strncpy
STUB_FUNC  0x4CF09BA2,scePaf_strcmp
STUB_FUNC  0xE0B32AE8,scePaf_strncmp
STUB_FUNC  0xC3B2D310,scePaf_strchr
STUB_FUNC  0x22420CC7,scePaf_strrchr
STUB_FUNC  0x4CE9C8D7,scePaf_strpbrk
STUB_FUNC  0xA4B8A4E3,scePaf_strtoul
STUB_END
