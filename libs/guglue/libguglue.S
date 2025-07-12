	.set noreorder

#include "pspimport.s"

// Build files
// libguglue_XXXX.o

#ifdef F_libguglue_0000
	IMPORT_START "guglue",0x00010000
#endif
#ifdef F_libguglue_0001
	IMPORT_FUNC  "guglue",0x050A7EB0,sceGuClutMode
#endif
#ifdef F_libguglue_0002
	IMPORT_FUNC  "guglue",0x29631777,sceGuClutLoad
#endif
#ifdef F_libguglue_0003
	IMPORT_FUNC  "guglue",0x7E0EADDE,sceGuEnable
#endif
#ifdef F_libguglue_0004
	IMPORT_FUNC  "guglue",0x3A4D566B,sceGuTexMode
#endif
#ifdef F_libguglue_0005
	IMPORT_FUNC  "guglue",0x7E5AED26,sceGuTexImage
#endif
#ifdef F_libguglue_0006
	IMPORT_FUNC  "guglue",0x16AB1034,sceGuTexFunc
#endif
#ifdef F_libguglue_0007
	IMPORT_FUNC  "guglue",0xBDEA3115,sceGuTexEnvColor
#endif
#ifdef F_libguglue_0008
	IMPORT_FUNC  "guglue",0xB07562AB,sceGuTexOffset
#endif
#ifdef F_libguglue_0009
	IMPORT_FUNC  "guglue",0x8053C305,sceGuTexWrap
#endif
#ifdef F_libguglue_0010
	IMPORT_FUNC  "guglue",0x3BA5C5CC,sceGuTexFilter
#endif
#ifdef F_libguglue_0011
	IMPORT_FUNC  "guglue",0xCCCC2EDC,sceGuGetMemory
#endif
#ifdef F_libguglue_0012
	IMPORT_FUNC  "guglue",0x4B4F39FC,sceGuDisable
#endif
#ifdef F_libguglue_0013
	IMPORT_FUNC  "guglue",0x5071E27A,sceGuDrawArray
#endif
#ifdef F_libguglue_0014
	IMPORT_FUNC  "guglue",0xB8730CB7,sceGuScissor
#endif
