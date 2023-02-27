	.set noreorder

#include "pspstub.s"

	STUB_START "idsRegeneration",0x40090000,0x00130005
	STUB_FUNC  0xBDE13E76,idsRegenerationSetup
	STUB_FUNC  0xFEE3C55B,idsRegenerationGetIndex
	STUB_FUNC  0x0B33639A,idsRegenerationGetHwConfigKeys
	STUB_FUNC  0xFEA979A6,idsRegenerationGetMGKeys
	STUB_FUNC  0xEE1AD992,idsRegenerationGetFactoryBadBlocksKey
	STUB_FUNC  0x87A30D3A,idsRegenerationGetSerialKey
	STUB_FUNC  0xE37CC2E6,idsRegenerationGetWlanKey
	STUB_FUNC  0xC44FE956,idsRegenerationGetAudioVolumeSetupKey
	STUB_FUNC  0xBC42FEED,idsRegenerationGetUsbKeys
	STUB_FUNC  0x3F014928,idsRegenerationGetUnkKey140
	STUB_FUNC  0x8F7EE9C0,idsRegenerationGetMGKey40
	STUB_FUNC  0xB9156F27,idsRegenerationGetUnkKeys3X
	STUB_FUNC  0xB417BCF0,idsRegenerationGetParentalLockKey
	STUB_FUNC  0xFA2368E8,idsRegenerationGenerateFactoryFirmwareKey
	STUB_FUNC  0x0EF8731D,idsRegenerationGetLCDKey
	STUB_FUNC  0x4E95D36F,idsRegenerationGenerateCallibrationKey
	STUB_FUNC  0xD7603D82,idsRegenerationGetUnkKeys5253
	STUB_FUNC  0x6015A7CD,idsRegenerationGetDefaultXMBColorKey
	STUB_FUNC  0xB79A6C46,idsRegenerationCreateCertificatesAndUMDKeys
	STUB_END
