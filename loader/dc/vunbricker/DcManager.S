	.set noreorder

#include "pspstub.s"

	STUB_START "DcManager",0x40090000,0x001B0005
	STUB_FUNC  0xA22190CE,dcGetHardwareInfo
	STUB_FUNC  0x3A3EA355,dcPatchModule
	STUB_FUNC  0xD298BA53,dcPatchModuleString
	STUB_FUNC  0x9C87A1C7,dcGetCancelMode
	STUB_FUNC  0xF5161120,dcSetCancelMode
	STUB_FUNC  0x39D91E83,dcLflashStartFatfmt
	STUB_FUNC  0xA899D713,dcLflashStartFDisk
	STUB_FUNC  0x741FFCFD,dcGetNandInfo
	STUB_FUNC  0xEA3F4DE5,dcLockNand
	STUB_FUNC  0x4A5FCC2F,dcUnlockNand
	STUB_FUNC  0x7EB5614A,dcReadNandBlock
	STUB_FUNC  0x504C82B4,dcWriteNandBlock
	STUB_FUNC  0x7E9128B0,dcEraseNandBlock
	STUB_FUNC  0xAD3509E5,dcRegisterPhysicalFormatCallback
	STUB_FUNC  0x1A718513,dcUnregisterPhysicalFormatCallback
	STUB_FUNC  0x48DDEA84,dcQueryRealMacAddress
	STUB_FUNC  0x767A728E,dcIdStorageUnformat
	STUB_FUNC  0xFE8EA2A0,dcIdStorageFormat
	STUB_FUNC  0xFFAAC913,dcIdStorageCreateLeaf
	STUB_FUNC  0xE5F8523D,dcIdStorageCreateAtomicLeaves
	STUB_FUNC  0x127323A6,dcIdStorageReadLeaf
	STUB_FUNC  0x549329DF,dcIdStorageWriteLeaf
	STUB_FUNC  0x34F3F7FC,dcIdStorageFlush
	STUB_FUNC  0xA6C4148A,dcSysconReceiveSetParam
	STUB_FUNC  0x3534B9C1,dcKirkCmd
	STUB_FUNC  0x109A36FC,SW
	STUB_FUNC  0x4914B8D1,LW
	STUB_END
