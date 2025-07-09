	.set noreorder

#include "pspstub.s"
	STUB_START "SysMemForKernel", 0x00010011, 0x000010005
	STUB_FUNC 0xCD617A94, sceKernelGetGameInfo
	STUB_END


#include "pspimport.s"
	IMPORT_START "SysclibForUser",0x40090000
	IMPORT_FUNC  "SysclibForUser",0x4C0FE24C,user_malloc