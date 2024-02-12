	.set noreorder

#include "pspstub.s"
	STUB_START "SysMemForKernel", 0x00010011, 0x000010005
	STUB_FUNC 0xCD617A94, sceKernelGetGameInfo
	STUB_END