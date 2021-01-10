	.set noreorder

#include "pspimport.s"

	IMPORT_START "SysMemUserForUser",0x40090000
	IMPORT_FUNC  "SysMemUserForUser",0xF77D77CB,sceKernelSetCompilerVersion
	IMPORT_FUNC  "SysMemUserForUser",0x91DE343C,sceKernelSetDevkitVersion
