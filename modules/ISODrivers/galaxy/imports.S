	.set noreorder

#include "pspstub.s"

	STUB_START "SysMemForKernel",0x40000011,0x00010005
	STUB_FUNC  0xC7E57B9C,sceKernelSetQTGP3
	STUB_END
