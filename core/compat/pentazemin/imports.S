	.set noreorder

#include "pspimport.s"
	
	IMPORT_START "SysMemForKernel",0x00010011
	IMPORT_FUNC  "SysMemForKernel",0x58148F07,sceKernelCreateHeap661
	IMPORT_FUNC  "SysMemForKernel",0x23D81675,sceKernelAllocHeapMemory661
	IMPORT_FUNC  "SysMemForKernel",0x87C2AB85,sceKernelFreeHeapMemory661
	IMPORT_FUNC  "SysMemForKernel",0x7158CE7E,sceKernelAllocPartitionMemory661
	IMPORT_FUNC  "SysMemForKernel",0xC1A26C6F,sceKernelFreePartitionMemory661
	IMPORT_FUNC  "SysMemForKernel",0xF12A62F7,sceKernelGetBlockHeadAddr661
	IMPORT_FUNC  "SysMemForKernel",0x83B5226D,sceKernelSetDdrMemoryProtection661
	IMPORT_FUNC  "SysMemForKernel",0x36C503A9,sceKernelGetSystemStatus661
	IMPORT_FUNC  "SysMemForKernel",0xEF29061C,sceKernelGetGameInfo661

	IMPORT_START "LoadCoreForKernel",0x00010011
	IMPORT_FUNC  "LoadCoreForKernel",0xF6B1BF0F,sceKernelFindModuleByName661
	IMPORT_FUNC  "LoadCoreForKernel",0xBC99C625,sceKernelFindModuleByAddress661
	IMPORT_FUNC  "LoadCoreForKernel",0x40972E6E,sceKernelFindModuleByUID661
	IMPORT_FUNC  "LoadCoreForKernel",0xD3353EC4,sceKernelCheckExecFile661
	IMPORT_FUNC  "LoadCoreForKernel",0x41D10899,sceKernelProbeExecutableObject661

	IMPORT_START "InterruptManagerForKernel",0x00010011
	IMPORT_FUNC  "InterruptManagerForKernel",0xF153B371,sceKernelQuerySystemCall661

	IMPORT_START "sceKermit_driver",0x00010011
	IMPORT_FUNC  "sceKermit_driver",0x36666181,sceKermitSendRequest661
	IMPORT_FUNC  "sceKermit_driver",0x5280B410,sceKermitRegisterVirtualIntrHandler661

	IMPORT_START "sceKermitMemory_driver",0x00090011
	IMPORT_FUNC  "sceKermitMemory_driver",0xAAF047AC,sceKermitMemorySetArgument661

	IMPORT_START "ModuleMgrForKernel",0x00010011
	IMPORT_FUNC  "ModuleMgrForKernel",0x1CF0B794,sceKernelLoadModuleBufferBootInitBtcnf661
	IMPORT_FUNC  "ModuleMgrForKernel",0x1CF0B794,sceKernelLoadModuleBuffer661
	IMPORT_FUNC  "ModuleMgrForKernel",0x2B7FC10D,sceKernelLoadModuleWithApitype2661
	IMPORT_FUNC  "ModuleMgrForKernel",0x7BD53193,sceKernelLoadModuleMs2661
	IMPORT_FUNC  "ModuleMgrForKernel",0x939E4270,sceKernelLoadModule661
	IMPORT_FUNC  "ModuleMgrForKernel",0x3FF74DF1,sceKernelStartModule661

	IMPORT_START "sceDisplay_driver",0x00090011
	IMPORT_FUNC  "sceDisplay_driver",0xA38B3F89,sceDisplaySetFrameBuf661

	IMPORT_START "LoadExecForKernel",0x00090011
	IMPORT_FUNC  "LoadExecForKernel",0x08F7166C,sceKernelExitVSHVSH661

	IMPORT_START "scePower_driver",0x00090011
	IMPORT_FUNC  "scePower_driver",0x9B44CFD9,scePowerRequestStandby661
	IMPORT_FUNC  "scePower_driver",0x5C1333B7,scePowerRequestSuspend661
	IMPORT_FUNC  "scePower_driver",0x9DAF25A0,scePowerRequestColdReset661
	IMPORT_FUNC  "scePower_driver",0x872F4ECE,scePowerIsPowerOnline661
	IMPORT_FUNC  "scePower_driver",0x7A9EA6DE,scePowerIsBatteryCharging661
	IMPORT_FUNC  "scePower_driver",0xC79F9157,scePowerGetBatteryLifeTime661
	IMPORT_FUNC  "scePower_driver",0x67492C52,scePowerGetBatteryChargingStatus661
	IMPORT_FUNC  "scePower_driver",0x40870DAC,scePowerGetBatteryTemp661
	IMPORT_FUNC  "scePower_driver",0xF7DE0E81,scePowerGetBatteryVolt661

	IMPORT_START "sceChkreg_driver",0x00090011
	IMPORT_FUNC  "sceChkreg_driver",0x54495B19,sceChkregCheckRegion661
	IMPORT_FUNC  "sceChkreg_driver",0x59F8491D,sceChkregGetPsCode661

	IMPORT_START "SystemCtrlPrivate",0x00090000
	IMPORT_FUNC  "SystemCtrlPrivate",0x5D7817CF,msstorCacheInit
	IMPORT_FUNC  "SystemCtrlPrivate",0xD31A2704,loadSettings
	IMPORT_FUNC  "SystemCtrlPrivate",0xBC7554DF,strcasecmp
	IMPORT_FUNC  "SystemCtrlPrivate",0x6EAFC03D,sctrlHENSetArkConfig

	IMPORT_START "SystemCtrlForKernel",0x00090000
	IMPORT_FUNC  "SystemCtrlForKernel",0x745286D1,sctrlHENSetMemory
	IMPORT_FUNC "SystemCtrlForKernel",0xFF73C3A3,exitLauncher