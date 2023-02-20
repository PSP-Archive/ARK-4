	.set noreorder

#include "pspimport.s"
	IMPORT_START "SystemCtrlPrivate",0x00090000
	IMPORT_FUNC  "SystemCtrlPrivate",0xD31A2704,loadSettings
	IMPORT_FUNC  "SystemCtrlPrivate",0x6EAFC03D,sctrlHENSetArkConfig


#include "pspstub.s"
	STUB_START "SysMemForKernel", 0x00010011, 0x000010005
	STUB_FUNC 0xCD617A94, sceKernelGetGameInfo
	STUB_END