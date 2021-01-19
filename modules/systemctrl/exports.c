#include <pspmoduleexport.h>
#define NULL ((void *) 0)

extern int module_start;
extern int module_info;
static const unsigned int __syslib_exports[4] __attribute__((section(".rodata.sceResident"))) = {
	0xD632ACDB,
	0xF01D73A7,
	(unsigned int) &module_start,
	(unsigned int) &module_info,
};

extern int sctrlKernelLoadExecVSHWithApitype;
extern int sctrlKernelSetUserLevel;
extern int sctrlKernelSetDevkitVersion;
extern int sctrlHENGetVersion;
extern int sctrlHENGetMinorVersion;
extern int sctrlHENFindFunction;
extern int sctrlSESetUmdFile;
extern int sctrlKernelSetInitApitype;
extern int sctrlKernelSetInitFileName;
extern int sctrlPatchModule;
extern int sctrlModuleTextAddr;
extern int sctrlKernelSetNidResolver;
extern int sctrlKernelRand;
extern int flushCache;
extern int sctrlSESetBootConfFileIndex;
extern int sctrlSEGetBootConfFileIndex;
extern int sctrlSESetBootConfFileIndex;
extern int sctrlSESetDiscType;
extern int sctrlSEGetDiscType;
extern int sctrlKernelGetGameID;
extern int sctrlGetThreadUIDByName;
extern int sctrlGetThreadContextByUID;
extern int sctrlGetThreadContextByName;
extern int sctrlDeflateDecompress;
extern int sctrlHENGetConfig;
extern int sctrlHENSetConfig;
extern int sctrlHENSetStartModuleHandler;
extern int printk;
extern int printkCached;
extern int printkSync;
extern int sctrlGetInitPARAM;
extern int sctrlHENGetArkConfig;
extern int sctrlHENSetPSXVramHandler;
extern int sctrlHENEnableCustomPeopsConfig;
extern int sctrlHENDisableCustomPeopsConfig;
static const unsigned int __SystemCtrlForUser_exports[70] __attribute__((section(".rodata.sceResident"))) = {
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
	0xF462EE55,
	0x71471E9D,
	0x9528E978,
	0x1C90BECB,
	0x3DF7F7D8,
	0xCB739F42,
	0x5C94CB48,
	0xFCE44FB8,
	0xFA82F439,
	0xA7D08A24,
	0x8823FF56,
	0xAD3E8705,
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
	(unsigned int) &sctrlDeflateDecompress,
	(unsigned int) &sctrlHENGetConfig,
	(unsigned int) &sctrlHENSetConfig,
	(unsigned int) &sctrlHENSetStartModuleHandler,
	(unsigned int) &printk,
	(unsigned int) &printkCached,
	(unsigned int) &printkSync,
	(unsigned int) &sctrlGetInitPARAM,
	(unsigned int) &sctrlHENGetArkConfig,
	(unsigned int) &sctrlHENSetPSXVramHandler,
	(unsigned int) &sctrlHENEnableCustomPeopsConfig,
	(unsigned int) &sctrlHENDisableCustomPeopsConfig,
};

extern int sctrlKernelLoadExecVSHWithApitype;
extern int sctrlKernelSetUserLevel;
extern int sctrlKernelSetDevkitVersion;
extern int sctrlHENGetVersion;
extern int sctrlHENGetMinorVersion;
extern int sctrlHENFindDriver;
extern int sctrlHENFindFunction;
extern int sctrlHENPatchSyscall;
extern int sctrlHENPatchSyscall;
extern int sctrlHENPatchSyscall;
extern int sctrlHENSetStartModuleHandler;
extern int sctrlHENFindFunction;
extern int oe_mallocinit;
extern int oe_malloc;
extern int oe_free;
extern int oe_mallocterminate;
extern int sctrlSEGetUmdFile;
extern int GetUmdFile;
extern int sctrlSESetUmdFile;
extern int SetUmdFile;
extern int sctrlKernelSetInitApitype;
extern int sctrlKernelSetInitFileName;
extern int sctrlPatchModule;
extern int sctrlModuleTextAddr;
extern int sctrlGetInitTextAddr;
extern int sctrlGetInitTextAddr;
extern int sctrlSetCustomStartModule;
extern int sctrlKernelSetNidResolver;
extern int sctrlKernelRand;
extern int findImportLib;
extern int findImportByNID;
extern int hookImportByNID;
extern int flushCache;
extern int sctrlKernelGetPSIDHash;
extern int printk;
extern int printkCached;
extern int printkSync;
extern int installJALTrace;
extern int installMemoryJALTrace;
extern int installModuleJALTrace;
extern int sctrlSESetBootConfFileIndex;
extern int sctrlSEGetBootConfFileIndex;
extern int sctrlSESetBootConfFileIndex;
extern int sctrlSESetDiscType;
extern int sctrlSEGetDiscType;
extern int sctrlKernelGetGameID;
extern int sctrlGetThreadUIDByName;
extern int sctrlGetThreadContextByUID;
extern int sctrlGetThreadContextByName;
extern int sctrlGetInitPARAM;
extern int sctrlHENGetArkConfig;
extern int sctrlHENSetPSXVramHandler;
static const unsigned int __SystemCtrlForKernel_exports[104] __attribute__((section(".rodata.sceResident"))) = {
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
	0xFCE44FB8,
	0xFA82F439,
	0xA7D08A24,
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
	(unsigned int) &sctrlGetInitPARAM,
	(unsigned int) &sctrlHENGetArkConfig,
	(unsigned int) &sctrlHENSetPSXVramHandler,
};

extern int kuKernelLoadModule;
extern int kuKernelInitApitype;
extern int kuKernelInitFileName;
extern int kuKernelInitKeyConfig;
extern int kuKernelBootFrom;
extern int kuKernelGetUserLevel;
extern int kuKernelSetDdrMemoryProtection;
extern int kuKernelGetModel;
extern int kuKernelPeekw;
extern int kuKernelPokew;
extern int kuKernelMemcpy;
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

extern int msstorCacheStat;
extern int msstorCacheDisable;
static const unsigned int __SystemCtrlPrivate_exports[4] __attribute__((section(".rodata.sceResident"))) = {
	0xFFC9D099,
	0x657301D9,
	(unsigned int) &msstorCacheStat,
	(unsigned int) &msstorCacheDisable,
};

extern int dumpJAL;
static const unsigned int __sceJumper_lib_exports[2] __attribute__((section(".rodata.sceResident"))) = {
	0x5F005E45,
	(unsigned int) &dumpJAL,
};

const struct _PspLibraryEntry __library_exports[6] __attribute__((section(".lib.ent"), used)) = {
	{ NULL, 0x0000, 0x8000, 4, 1, 1, (unsigned int *) &__syslib_exports },
	{ "SystemCtrlForUser", 0x0000, 0x4001, 4, 0, 35, (unsigned int *) &__SystemCtrlForUser_exports },
	{ "SystemCtrlForKernel", 0x0000, 0x0001, 4, 0, 52, (unsigned int *) &__SystemCtrlForKernel_exports },
	{ "KUBridge", 0x0000, 0x4001, 4, 0, 11, (unsigned int *) &__KUBridge_exports },
	{ "SystemCtrlPrivate", 0x0000, 0x0001, 4, 0, 2, (unsigned int *) &__SystemCtrlPrivate_exports },
	{ "sceJumper_lib", 0x0000, 0x4001, 4, 0, 1, (unsigned int *) &__sceJumper_lib_exports },
};
