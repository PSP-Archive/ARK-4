	.set noreorder

#include "pspstub.s"

	STUB_START "pspPSAR",0x40090000,0x00030005
	STUB_FUNC  0x56C924E1,pspPSARInit
	STUB_FUNC  0x9C91DD9D,pspPSARGetNextFile
	STUB_FUNC  0xC907D012,pspPSARSetBufferPosition
	STUB_END
