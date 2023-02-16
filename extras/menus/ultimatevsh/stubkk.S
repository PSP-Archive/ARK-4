	.set noreorder

#include "pspstub.s"

STUB_START "SystemCtrlForUser",0x40090000,0x00020005 
STUB_FUNC 0x1DDDAD0C, sctrlSESetConfig
STUB_FUNC 0x16C3B7EE, sctrlSEGetConfig
STUB_END 

STUB_START "scePower", 0x40010000, 0x00030005
STUB_FUNC 0x2B7C7CF4, scePowerRequestStandby
STUB_FUNC 0x0442D852, scePowerRequestColdReset
STUB_FUNC 0xAC32C9CC, scePowerRequestSuspend
STUB_END

STUB_START "VshCtrlLib",0x40090000,0x00030005 
STUB_FUNC 0xFD26DA72, vctrlVSHRegisterVshMenu
STUB_FUNC 0xCD6B3913, vctrlVSHExitVSHMenu
STUB_FUNC 0xC1FED158, vshDetectDiscType
STUB_END 

STUB_START "scePaf",0x00010011,0x00150005 
STUB_FUNC 0x49A72E5D, scePaf_strlen
STUB_FUNC 0x58189108, scePaf_strlen_620
STUB_FUNC 0xD7DCB972, scePaf_strlen_660
STUB_FUNC 0x43B6A88A, scePaf_memset
STUB_FUNC 0x6F64AB9B, scePaf_memset_620
STUB_FUNC 0xBB89C9EA, scePaf_memset_660
STUB_FUNC 0x861C4627, scePaf_memcmp
STUB_FUNC 0x16789955, scePaf_memcmp_620
STUB_FUNC 0xB05D9677, scePaf_memcmp_660
STUB_FUNC 0x4C386F3C, scePaf_sprintf
STUB_FUNC 0xE8473E80, scePaf_sprintf_620
STUB_FUNC 0xA138A376, scePaf_sprintf_660
STUB_FUNC 0x726776D7, scePaf_snprintf
STUB_FUNC 0x5E7610DF, scePaf_snprintf_620
STUB_FUNC 0x7B7133D5, scePaf_snprintf_660
STUB_FUNC 0xE281261E, scePaf_memcpy
STUB_FUNC 0x23C8DAB5, scePaf_memcpy_620
STUB_FUNC 0xF7C46E37, scePaf_memcpy_660
STUB_FUNC 0x8F95CC01, scePaf_strcpy
STUB_FUNC 0x4F487FBC, scePaf_strcpy_620
STUB_FUNC 0x726DFBA9, scePaf_strcpy_660
STUB_END 
