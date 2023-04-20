	.set noreorder

#include "pspstub.s"

	STUB_START "LflashFdisk",0x00090000,0x00010005
	STUB_FUNC  0x7A34548C,pspLflashFdiskStartFdisk
	STUB_END
