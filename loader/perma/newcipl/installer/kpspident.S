	.set noreorder

#include "pspstub.s"

	STUB_START "kpspident",0x40090000,0x00050005
	STUB_FUNC  0x7EC5A957,sceSysconGetBaryonVersion
	STUB_FUNC  0xE7E87741,sceSysconGetPommelVersion
	STUB_FUNC  0xE2A5D1EE,sceSysregGetTachyonVersion
	STUB_FUNC  0xA27B1BA6,pspGetKirkVersion
	STUB_FUNC  0xFF2702F2,pspGetSpockVersion
	STUB_END
