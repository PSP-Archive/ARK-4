	.set noreorder

#include "pspstub.s"

	STUB_START "SysMemForKernel",0x00000011,0x00050005
	STUB_FUNC  0xC7E57B9C,sceKernelSetQTGP3
	STUB_FUNC  0xFC114573,sceKernelGetCompiledSdkVersion
	STUB_FUNC  0xCF4DE78C,sceKernelGetUIDcontrolBlock
	STUB_END
