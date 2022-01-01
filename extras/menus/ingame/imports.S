	.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForUser",0x40090000,0x00030005
	STUB_FUNC  0xAA6274F8,sctrlGetThreads
	STUB_FUNC  0x373C8373,sctrlSuspendThreads
	STUB_FUNC  0xE2A48BE5,sctrlResumeThreads
	STUB_END

	STUB_START "sceDmac",0x40010000,0x00010005
	STUB_FUNC  0x617F3FE6,sceDmacMemcpy
	STUB_END
