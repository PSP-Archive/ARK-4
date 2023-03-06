	.set noreorder

#include "pspimport.s"

	IMPORT_START "sceNand_driver",0x00010000
	IMPORT_FUNC	"sceNand_driver",0xAE4438C7,sceNandLock
	IMPORT_FUNC	"sceNand_driver",0x41FFA822,sceNandUnlock
	IMPORT_FUNC	"sceNand_driver",0x0ADC8686,sceNandWriteAccess
	IMPORT_FUNC	"sceNand_driver",0xEB0A0022,sceNandEraseBlock
	IMPORT_FUNC	"sceNand_driver",0x5182C394,sceNandReadExtraOnly
	IMPORT_FUNC	"sceNand_driver",0xCE9843E6,sceNandGetPageSize
	IMPORT_FUNC	"sceNand_driver",0xB07C41D4,sceNandGetPagesPerBlock
	IMPORT_FUNC	"sceNand_driver",0xC1376222,sceNandGetTotalBlocks
	IMPORT_FUNC	"sceNand_driver",0x01F09203,sceNandIsBadBlock
	IMPORT_FUNC	"sceNand_driver",0xC478C1DE,sceNandReadPagesRawAll
	IMPORT_FUNC	"sceNand_driver",0xB2B021E5,sceNandWriteBlockWithVerify
	IMPORT_FUNC	"sceNand_driver",0xC32EA051,sceNandReadBlockWithRetry
	IMPORT_FUNC "sceNand_driver",0x8933B2E0,sceNandEraseBlockWithRetry
