	.set noreorder

#include "pspstub.s"

STUB_START "scePower", 0x40010000, 0x00030005
STUB_FUNC 0x2B7C7CF4, scePowerRequestStandby
STUB_FUNC 0x0442D852, scePowerRequestColdReset
STUB_FUNC 0xAC32C9CC, scePowerRequestSuspend
STUB_END

STUB_START "VshCtrlLib",0x40090000,0x00030005 
STUB_FUNC 0xFD26DA72, vctrlVSHRegisterVshMenu
STUB_FUNC 0xCD6B3913, vctrlVSHExitVSHMenu
STUB_END 
