	.set noreorder

#include "pspstub.s"
	STUB_START "SysMemForKernel", 0x00010011, 0x000010005
	STUB_FUNC 0xCD617A94, sceKernelGetGameInfo
	STUB_END

	STUB_START "sceMpeg", 0x40090011, 0x00030005
	STUB_FUNC 0xD8C5F121, sceMpegCreate
	STUB_END