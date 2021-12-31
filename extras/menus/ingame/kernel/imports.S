	.set noreorder

#include "pspstub.s"

	STUB_START	"SysMemForKernel",0x00010011,0x00010005
	STUB_FUNC	0xEF29061C,sceKernelGetGameInfo
	STUB_END

	STUB_START	"sceKermit_driver",0x00010011,0x00010005
	STUB_FUNC	0x351B5CC4,sceKermitIsDone
	STUB_END
	
	STUB_START	"sceSasCore_driver",0x00090011,0x00020005
	STUB_FUNC	0xB0F9F98F,sceSasCoreInit
	STUB_FUNC	0xE143A1EA,sceSasCoreExit
	STUB_END