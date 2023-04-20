	.set noreorder

#include "pspstub.s"

	STUB_START "pspDecrypt",0x40090000,0x00070005
	STUB_FUNC  0xB081FD37,pspDecryptPRX
	STUB_FUNC  0x10360D7D,pspSignCheck
	STUB_FUNC  0xE1826CB7,pspDecryptIPL1
	STUB_FUNC  0x40ADB206,pspLinearizeIPL2
	STUB_FUNC  0xDCE318A9,pspDecryptIPL3
	STUB_FUNC  0x018C3EAC,pspDecompress
	STUB_FUNC  0xD31CB80A,pspDecryptTable
	STUB_END