	.set noreorder

#include "pspstub.s"

	STUB_START "SaveStateLibrary",0x40090000,0x00030005
	STUB_FUNC  0xAA6274F8,GetThreads
	STUB_FUNC  0x373C8373,SuspendThreads
	STUB_FUNC  0xE2A48BE5,ResumeThreads
	STUB_END

	STUB_START "sceDmac",0x40010000,0x00010005
	STUB_FUNC  0x617F3FE6,sceDmacMemcpy
	STUB_END
	
	STUB_START "SysMemUserForUser",0x40000000,0x00010005
	STUB_FUNC  0x237DBD4F,_sceKernelAllocPartitionMemory
	STUB_END