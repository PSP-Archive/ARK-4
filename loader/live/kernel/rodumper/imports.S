	.set noreorder

#include "pspstub.s"

	STUB_START "sceNetIfhandle_lib",0x40090011,0x00010005
	STUB_FUNC  0x1560F143,sceNetMCopyback
	STUB_END
