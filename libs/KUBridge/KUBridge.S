	.set noreorder

#include "pspimport.s"

// Build files
// KUBridge_XXXX.o

#ifdef F_KUBridge_0000
	IMPORT_START "KUBridge",0x40090000
#endif
#ifdef F_KUBridge_0001
	IMPORT_FUNC  "KUBridge",0x4C25EA72,kuKernelLoadModule
#endif
#ifdef F_KUBridge_0002
	IMPORT_FUNC  "KUBridge",0x8E5A4057,kuKernelInitApitype
#endif
#ifdef F_KUBridge_0003
	IMPORT_FUNC  "KUBridge",0x1742445F,kuKernelInitFileName
#endif
#ifdef F_KUBridge_0004
	IMPORT_FUNC  "KUBridge",0xB0B8824E,kuKernelInitKeyConfig
#endif
#ifdef F_KUBridge_0005
	IMPORT_FUNC  "KUBridge",0x60DDB4AE,kuKernelBootFrom
#endif
#ifdef F_KUBridge_0006
	IMPORT_FUNC  "KUBridge",0xA2ABB6D3,kuKernelGetUserLevel
#endif
#ifdef F_KUBridge_0007
	IMPORT_FUNC  "KUBridge",0xC4AF12AB,kuKernelSetDdrMemoryProtection
#endif
#ifdef F_KUBridge_0008
	IMPORT_FUNC  "KUBridge",0x24331850,kuKernelGetModel
#endif
#ifdef F_KUBridge_0009
	IMPORT_FUNC  "KUBridge",0x7A50075E,kuKernelPeekw
#endif
#ifdef F_KUBridge_0010
	IMPORT_FUNC  "KUBridge",0x0E73A39D,kuKernelPokew
#endif
#ifdef F_KUBridge_0011
	IMPORT_FUNC  "KUBridge",0x6B4B577F,kuKernelMemcpy
#endif
#ifdef F_KUBridge_0012
	IMPORT_FUNC  "KUBridge",0xD0D05A5B,kuKernelGetUmdFile
#endif
#ifdef F_KUBridge_0013
	IMPORT_FUNC  "KUBridge",0x1E9F0498,kuKernelLoadModuleWithApitype2
#endif
#ifdef F_KUBridge_0014
	IMPORT_FUNC  "KUBridge",0x4B321167,kuKernelFindModuleByName
#endif
#ifdef F_KUBridge_0015
	IMPORT_FUNC  "KUBridge",0x9060F69D,kuKernelCall
#endif
#ifdef F_KUBridge_0016
	IMPORT_FUNC  "KUBridge",0x5C6C3DBA,kuKernelCallExtendStack
#endif
#ifdef F_KUBridge_0017
	IMPORT_FUNC  "KUBridge",0x219DE4D2,kuKernelIcacheInvalidateAll
#endif
#ifdef F_KUBridge_0018
	IMPORT_FUNC  "KUBridge",0x501E983D,kuKernelInitMode
#endif
