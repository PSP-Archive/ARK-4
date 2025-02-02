#ifndef __DCMAN_H__
#define __DCMAN_H__

enum
{
    TA_079v1,
    TMU_001v1,
    TA_079v2,
    TMU_001v2,
    TA_079v3,
    TMU_002,
    TA_079v4,
    TA_079v5,
    TA_081v1,
    TA_081v2,
    TA_082,
    TA_086,
    TA_085v1,
    TA_085v2,
    TA_088v1_TA_088v2,
    TA_090v1,
    TA_088v3,
    TA_090v2,
    TA_090v3,
    TA_092,
    TA_091,
    TA_094,
    TA_093v1,
    TA_093v2,
    TA_095v1,
    TA_095v2,
    TA_095v3,
    TA_095v4,
    TA_096_TA_097,
    UNKNOWN
};

int dcGetHardwareInfo(u32 *ptachyon, u32 *pbaryon, u32 *ppommel, u32 *pmb, u64 *pfuseid, u32 *pfuseconfig, u32 *pnandsize);
int dcPatchModule(char *modname, int type, u32 addr, u32 word);
int dcPatchModuleString(char *modname, char *string, char *replace);
int dcGetCancelMode();
int dcSetCancelMode(int mode);
int dcLflashStartFatfmt(int argc, char *argv[]);
int dcLflashStartFDisk(int argc, char *argv[]);
int dcGetNandInfo(u32 *pagesize, u32 *ppb, u32 *totalblocks);
int dcLockNand(int flag);
int dcUnlockNand();
int dcReadNandBlock(u32 page, u8 *block);
int dcWriteNandBlock(u32 page, u8 *user, u8 *spare);
int dcEraseNandBlock(u32 page);
int dcRegisterPhysicalFormatCallback(SceUID cbid);
int dcUnregisterPhysicalFormatCallback();
int dcQueryRealMacAddress(u8 *mac);
int dcIdStorageUnformat();
int dcIdStorageFormat();
int dcIdStorageCreateLeaf(u16 leafid);
int dcIdStorageCreateAtomicLeaves(u16 *leaves, int n);
int dcIdStorageReadLeaf(u16 leafid, u8 *buf);
int dcIdStorageWriteLeaf(u16 leafid, u8 *buf);
int dcIdStorageFlush();
int dcSysconReceiveSetParam(int, u8 *);
int dcKirkCmd(u8 *in, u32 insize, u8 *out, u32 outsize, int cmd);

void SW(u32 word, u32 address);
u32  LW(u32 address);

#endif

