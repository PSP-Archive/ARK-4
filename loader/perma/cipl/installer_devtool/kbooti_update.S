	.set noreorder

#include "pspstub.s"

	STUB_START "kbooti_update",0x40090000,0x00030005
	STUB_FUNC  0x8B650F57,pspKbootiUpdateGetKbootiSize
	STUB_FUNC  0x2FBA6BE4,pspKbootiUpdateKbooti
	STUB_FUNC  0x32A8C95A,pspKbootiUpdateRestoreKbooti
	STUB_END