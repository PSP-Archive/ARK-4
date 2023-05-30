	.set noreorder

#include "pspimport.s"

    IMPORT_START "SystemCtrlPrivate",0x00090000
	IMPORT_FUNC  "SystemCtrlPrivate",0x5D7817CF,msstorCacheInit
	IMPORT_FUNC  "SystemCtrlPrivate",0xBC7554DF,strcasecmp

	IMPORT_START "SystemCtrlForKernel",0x00090000
	IMPORT_FUNC  "SystemCtrlForKernel",0x745286D1,sctrlHENSetMemory
	IMPORT_FUNC "SystemCtrlForKernel",0xFF73C3A3,exitLauncher
