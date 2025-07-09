	.set noreorder

#include "pspstub.s"

STUB_START "scePower", 0x40010000, 0x00030005
STUB_FUNC 0x2B7C7CF4, scePowerRequestStandby
STUB_FUNC 0x0442D852, scePowerRequestColdReset
STUB_FUNC 0xAC32C9CC, scePowerRequestSuspend
STUB_END

STUB_START "SysMemUserForUser", 0x40000011, 0x00030005
STUB_FUNC 0x7591C7DB, sceKernelSetCompiledSdkVersion
STUB_END

#include "pspimport.s"

IMPORT_START "unarchive", 0x40090000
IMPORT_FUNC "unarchive", 0xF6EC2A3A, unarchiveFile
