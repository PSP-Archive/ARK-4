#include <pspmoduleexport.h>
#define NULL ((void *) 0)

extern void module_bootstart;
extern void module_start;
extern void module_info;
static const unsigned int __syslib_exports[6] __attribute__((section(".rodata.sceResident"))) = {
	0xD3744BE0,
	0xD632ACDB,
	0xF01D73A7,
	(unsigned int) &module_bootstart,
	(unsigned int) &module_start,
	(unsigned int) &module_info,
};

extern void sctrlKernelLoadExecVSHWithApitype;
extern void sctrlKernelSetUserLevel;
extern void sctrlKernelSetDevkitVersion;
extern void sctrlHENGetVersion;
extern void sctrlHENGetMinorVersion;
extern void sctrlHENFindFunction;
extern void sctrlSESetUmdFile;
extern void sctrlKernelSetInitApitype;
extern void sctrlKernelSetInitFileName;
extern void sctrlPatchModule;
extern void sctrlModuleTextAddr;
extern void sctrlKernelSetNidResolver;
extern void sctrlKernelRand;
extern void flushCache;
extern void sctrlSESetBootConfFileIndex;
extern void sctrlSEGetBootConfFileIndex;
extern void sctrlSESetBootConfFileIndex;
extern void sctrlSESetDiscType;
extern void sctrlSEGetDiscType;
extern void sctrlKernelGetGameID;
extern void sctrlGetThreadUIDByName;
extern void sctrlGetThreadContextByUID;
extern void sctrlGetThreadContextByName;
static const unsigned int __SystemCtrlForUser_exports[46] __attribute__((section(".rodata.sceResident"))) = {
	0x2D10FB28,
	0xEB74FE45,
	0xD8FF9B99,
	0x1090A2E1,
	0x5328B431,
	0x159AF5CC,
	0x5A35C948,
	0x8D5BE1F0,
	0x128112C3,
	0x62CAC4CF,
	0x2A868045,
	0x603EE1D0,
	0xB364FBB4,
	0x324DF3DD,
	0xBC939DC1,
	0x70B92B45,
	0x5CB025F0,
	0x31C6160D,
	0xABEF849B,
	0x969306E7,
	0x23833651,
	0xF93BEC5A,
	0x72D520D4,
	(unsigned int) &sctrlKernelLoadExecVSHWithApitype,
	(unsigned int) &sctrlKernelSetUserLevel,
	(unsigned int) &sctrlKernelSetDevkitVersion,
	(unsigned int) &sctrlHENGetVersion,
	(unsigned int) &sctrlHENGetMinorVersion,
	(unsigned int) &sctrlHENFindFunction,
	(unsigned int) &sctrlSESetUmdFile,
	(unsigned int) &sctrlKernelSetInitApitype,
	(unsigned int) &sctrlKernelSetInitFileName,
	(unsigned int) &sctrlPatchModule,
	(unsigned int) &sctrlModuleTextAddr,
	(unsigned int) &sctrlKernelSetNidResolver,
	(unsigned int) &sctrlKernelRand,
	(unsigned int) &flushCache,
	(unsigned int) &sctrlSESetBootConfFileIndex,
	(unsigned int) &sctrlSEGetBootConfFileIndex,
	(unsigned int) &sctrlSESetBootConfFileIndex,
	(unsigned int) &sctrlSESetDiscType,
	(unsigned int) &sctrlSEGetDiscType,
	(unsigned int) &sctrlKernelGetGameID,
	(unsigned int) &sctrlGetThreadUIDByName,
	(unsigned int) &sctrlGetThreadContextByUID,
	(unsigned int) &sctrlGetThreadContextByName,
};

extern void sctrlKernelLoadExecVSHWithApitype;
extern void sctrlKernelSetUserLevel;
extern void sctrlKernelSetDevkitVersion;
extern void sctrlHENGetVersion;
extern void sctrlHENGetMinorVersion;
extern void sctrlHENFindDriver;
extern void sctrlHENFindFunction;
extern void sctrlHENPatchSyscall;
extern void sctrlHENPatchSyscall;
extern void sctrlHENPatchSyscall;
extern void sctrlHENSetStartModuleHandler;
extern void sctrlHENFindFunction;
extern void oe_mallocinit;
extern void oe_malloc;
extern void oe_free;
extern void oe_mallocterminate;
extern void sctrlSEGetUmdFile;
extern void GetUmdFile;
extern void sctrlSESetUmdFile;
extern void SetUmdFile;
extern void sctrlKernelSetInitApitype;
extern void sctrlKernelSetInitFileName;
extern void sctrlPatchModule;
extern void sctrlModuleTextAddr;
extern void sctrlGetInitTextAddr;
extern void sctrlGetInitTextAddr;
extern void sctrlSetCustomStartModule;
extern void sctrlKernelSetNidResolver;
extern void sctrlKernelRand;
extern void findImportLib;
extern void findImportByNID;
extern void hookImportByNID;
extern void flushCache;
extern void sctrlKernelGetPSIDHash;
extern void printk;
extern void printkCached;
extern void printkSync;
extern void installJALTrace;
extern void installMemoryJALTrace;
extern void installModuleJALTrace;
extern void sctrlSESetBootConfFileIndex;
extern void sctrlSEGetBootConfFileIndex;
extern void sctrlSESetBootConfFileIndex;
extern void sctrlSESetDiscType;
extern void sctrlSEGetDiscType;
extern void sctrlKernelGetGameID;
extern void sctrlGetThreadUIDByName;
extern void sctrlGetThreadContextByUID;
extern void sctrlGetThreadContextByName;
static const unsigned int __SystemCtrlForKernel_exports[98] __attribute__((section(".rodata.sceResident"))) = {
	0x2D10FB28,
	0xEB74FE45,
	0xD8FF9B99,
	0x1090A2E1,
	0x5328B431,
	0x78E46415,
	0x159AF5CC,
	0xF988C1DC,
	0x826668E9,
	0x02BFCB5F,
	0x1C90BECB,
	0x159AF5CC,
	0x1E5436EE,
	0xF9584CAD,
	0xA65E8BC4,
	0xE34A0D97,
	0xBA21998E,
	0xAC56B90B,
	0x5A35C948,
	0xB64186D0,
	0x8D5BE1F0,
	0x128112C3,
	0x62CAC4CF,
	0x2A868045,
	0x557F0B8C,
	0x72F29A6E,
	0x259B51CE,
	0x603EE1D0,
	0xB364FBB4,
	0x028687B4,
	0x16C3137E,
	0x869F24E9,
	0x324DF3DD,
	0xCE2F0F74,
	0x3DF7F7D8,
	0xCB739F42,
	0x5C94CB48,
	0x68363B69,
	0xBB365B42,
	0x804572A3,
	0xBC939DC1,
	0x70B92B45,
	0x5CB025F0,
	0x31C6160D,
	0xABEF849B,
	0x969306E7,
	0x23833651,
	0xF93BEC5A,
	0x72D520D4,
	(unsigned int) &sctrlKernelLoadExecVSHWithApitype,
	(unsigned int) &sctrlKernelSetUserLevel,
	(unsigned int) &sctrlKernelSetDevkitVersion,
	(unsigned int) &sctrlHENGetVersion,
	(unsigned int) &sctrlHENGetMinorVersion,
	(unsigned int) &sctrlHENFindDriver,
	(unsigned int) &sctrlHENFindFunction,
	(unsigned int) &sctrlHENPatchSyscall,
	(unsigned int) &sctrlHENPatchSyscall,
	(unsigned int) &sctrlHENPatchSyscall,
	(unsigned int) &sctrlHENSetStartModuleHandler,
	(unsigned int) &sctrlHENFindFunction,
	(unsigned int) &oe_mallocinit,
	(unsigned int) &oe_malloc,
	(unsigned int) &oe_free,
	(unsigned int) &oe_mallocterminate,
	(unsigned int) &sctrlSEGetUmdFile,
	(unsigned int) &GetUmdFile,
	(unsigned int) &sctrlSESetUmdFile,
	(unsigned int) &SetUmdFile,
	(unsigned int) &sctrlKernelSetInitApitype,
	(unsigned int) &sctrlKernelSetInitFileName,
	(unsigned int) &sctrlPatchModule,
	(unsigned int) &sctrlModuleTextAddr,
	(unsigned int) &sctrlGetInitTextAddr,
	(unsigned int) &sctrlGetInitTextAddr,
	(unsigned int) &sctrlSetCustomStartModule,
	(unsigned int) &sctrlKernelSetNidResolver,
	(unsigned int) &sctrlKernelRand,
	(unsigned int) &findImportLib,
	(unsigned int) &findImportByNID,
	(unsigned int) &hookImportByNID,
	(unsigned int) &flushCache,
	(unsigned int) &sctrlKernelGetPSIDHash,
	(unsigned int) &printk,
	(unsigned int) &printkCached,
	(unsigned int) &printkSync,
	(unsigned int) &installJALTrace,
	(unsigned int) &installMemoryJALTrace,
	(unsigned int) &installModuleJALTrace,
	(unsigned int) &sctrlSESetBootConfFileIndex,
	(unsigned int) &sctrlSEGetBootConfFileIndex,
	(unsigned int) &sctrlSESetBootConfFileIndex,
	(unsigned int) &sctrlSESetDiscType,
	(unsigned int) &sctrlSEGetDiscType,
	(unsigned int) &sctrlKernelGetGameID,
	(unsigned int) &sctrlGetThreadUIDByName,
	(unsigned int) &sctrlGetThreadContextByUID,
	(unsigned int) &sctrlGetThreadContextByName,
};

extern void kuKernelLoadModule;
extern void kuKernelInitApitype;
extern void kuKernelInitFileName;
extern void kuKernelInitKeyConfig;
extern void kuKernelBootFrom;
extern void kuKernelGetUserLevel;
extern void kuKernelSetDdrMemoryProtection;
extern void kuKernelGetModel;
extern void kuKernelPeekw;
extern void kuKernelPokew;
extern void kuKernelMemcpy;
static const unsigned int __KUBridge_exports[22] __attribute__((section(".rodata.sceResident"))) = {
	0x4C25EA72,
	0x8E5A4057,
	0x1742445F,
	0xB0B8824E,
	0x60DDB4AE,
	0xA2ABB6D3,
	0xC4AF12AB,
	0x24331850,
	0x7A50075E,
	0x0E73A39D,
	0x6B4B577F,
	(unsigned int) &kuKernelLoadModule,
	(unsigned int) &kuKernelInitApitype,
	(unsigned int) &kuKernelInitFileName,
	(unsigned int) &kuKernelInitKeyConfig,
	(unsigned int) &kuKernelBootFrom,
	(unsigned int) &kuKernelGetUserLevel,
	(unsigned int) &kuKernelSetDdrMemoryProtection,
	(unsigned int) &kuKernelGetModel,
	(unsigned int) &kuKernelPeekw,
	(unsigned int) &kuKernelPokew,
	(unsigned int) &kuKernelMemcpy,
};

extern void msstorCacheStat;
extern void msstorCacheDisable;
static const unsigned int __SystemCtrlPrivate_exports[4] __attribute__((section(".rodata.sceResident"))) = {
	0xFFC9D099,
	0x657301D9,
	(unsigned int) &msstorCacheStat,
	(unsigned int) &msstorCacheDisable,
};

extern void sctrlKernelDummyFunction;
extern void sctrlKernelDummyFunction;
extern void sctrlKernelDummyFunction;
static const unsigned int __sceSyscon_driver_exports[6] __attribute__((section(".rodata.sceResident"))) = {
	0x48448373,
	0x8CBC7987,
	0x9BC5E33B,
	(unsigned int) &sctrlKernelDummyFunction,
	(unsigned int) &sctrlKernelDummyFunction,
	(unsigned int) &sctrlKernelDummyFunction,
};

extern void sctrlKernelDummyFunction;
static const unsigned int __sceLFatFs_driver_exports[2] __attribute__((section(".rodata.sceResident"))) = {
	0x933F6E29,
	(unsigned int) &sctrlKernelDummyFunction,
};

extern void sctrlKernelDummyFunction;
static const unsigned int __sceClockgen_driver_exports[2] __attribute__((section(".rodata.sceResident"))) = {
	0xDAB6E612,
	(unsigned int) &sctrlKernelDummyFunction,
};

extern void sctrlKernelDummyFunction;
static const unsigned int __sceCodec_driver_exports[2] __attribute__((section(".rodata.sceResident"))) = {
	0x376399B6,
	(unsigned int) &sctrlKernelDummyFunction,
};

extern void dumpJAL;
static const unsigned int __sceJumper_lib_exports[2] __attribute__((section(".rodata.sceResident"))) = {
	0x5F005E45,
	(unsigned int) &dumpJAL,
};

const struct _PspLibraryEntry __library_exports[10] __attribute__((section(".lib.ent"), used)) = {
	{ NULL, 0x0000, 0x8000, 4, 1, 2, &__syslib_exports },
	{ "SystemCtrlForUser", 0x0000, 0x4001, 4, 0, 23, &__SystemCtrlForUser_exports },
	{ "SystemCtrlForKernel", 0x0000, 0x0001, 4, 0, 49, &__SystemCtrlForKernel_exports },
	{ "KUBridge", 0x0000, 0x4001, 4, 0, 11, &__KUBridge_exports },
	{ "SystemCtrlPrivate", 0x0000, 0x0001, 4, 0, 2, &__SystemCtrlPrivate_exports },
	{ "sceSyscon_driver", 0x0011, 0x0001, 4, 0, 3, &__sceSyscon_driver_exports },
	{ "sceLFatFs_driver", 0x0011, 0x0001, 4, 0, 1, &__sceLFatFs_driver_exports },
	{ "sceClockgen_driver", 0x0011, 0x0001, 4, 0, 1, &__sceClockgen_driver_exports },
	{ "sceCodec_driver", 0x0011, 0x0001, 4, 0, 1, &__sceCodec_driver_exports },
	{ "sceJumper_lib", 0x0000, 0x4001, 4, 0, 1, &__sceJumper_lib_exports },
};
