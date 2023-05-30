	.set noreorder

#include "pspimport.s"

	IMPORT_START	"InitForKernel",0x00090000
	IMPORT_FUNC	"InitForKernel",0x27932388,sceKernelBootFrom
	IMPORT_FUNC	"InitForKernel",0x7233B5BC,sceKernelApplicationType
	IMPORT_FUNC	"InitForKernel",0x7A2333AD,sceKernelInitApitype
	IMPORT_FUNC	"InitForKernel",0xA6E71B93,sceKernelInitFileName
	IMPORT_FUNC	"InitForKernel",0x040C934B,InitForKernel_040C934B

	IMPORT_START "SystemCtrlPrivate",0x00090000
	IMPORT_FUNC  "SystemCtrlPrivate",0x5D7817CF,msstorCacheInit
	IMPORT_FUNC  "SystemCtrlPrivate",0xBC7554DF,strcasecmp
	IMPORT_FUNC  "SystemCtrlPrivate",0xAC7554DF,strncasecmp
	IMPORT_FUNC  "SystemCtrlPrivate",0x77E68D5F,sctrlHENSetPluginHandler
	
	IMPORT_START "SystemCtrlForKernel",0x00090000
	IMPORT_FUNC  "SystemCtrlForKernel",0x745286D1,sctrlHENSetMemory
