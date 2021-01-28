	.set noreorder

#include "pspstub.s"

	STUB_START "SysMemForKernel",0x40000011,0x00010005
	STUB_FUNC  0xC7E57B9C,sceKernelSetQTGP3
	STUB_END

#include "pspimport.s"	
	IMPORT_START "SystemCtrlForKernel",0x00090000
	IMPORT_FUNC  "SystemCtrlForKernel",0x16100529,LZ4_decompress_fast
