	.set noreorder

#include "pspstub.s"

	STUB_START "SysMemForKernel",0x00000011,0x00050005
	STUB_FUNC  0xC7E57B9C,sceKernelSetQTGP3
	STUB_FUNC  0xFC114573,sceKernelGetCompiledSdkVersion
	STUB_FUNC  0xCF4DE78C,sceKernelGetUIDcontrolBlock
	STUB_FUNC  0x3FC9AE6A,sceKernelDevkitVersion
	STUB_FUNC  0x6373995D,sceKernelGetModel
	STUB_END

	STUB_START "InitForKernel",0x00090011,0x00030005
	STUB_FUNC  0xEE67E450,sceKernelBootFromGo635
	STUB_FUNC  0x977A0F90,sceKernelBootFromGo620
	STUB_FUNC  0x7233B5BC,sceKernelApplicationType
	STUB_END

#include "pspimport.s"	
	IMPORT_START "SystemCtrlForKernel",0x00090000
	IMPORT_FUNC  "SystemCtrlForKernel",0x16100529,LZ4_decompress_fast
