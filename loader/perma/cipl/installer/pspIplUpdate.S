	.set noreorder

#include "pspstub.s"

	STUB_START "pspIplUpdate",0x40090000,0x00030005
	STUB_FUNC  0xAD923E2F,pspIplUpdateGetIpl
	STUB_FUNC  0xF7EF8E3F,pspIplUpdateSetIpl
	STUB_FUNC  0x282EDFEC,pspIplUpdateClearIpl
	STUB_END
