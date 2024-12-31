#ifndef __IDSREGENERATION_H__
#define __IDSREGENERATION_H__

typedef struct IdsIndex
{
    int keyfirst;
    int keyend;
} IdsIndex;

int idsRegenerationSetup(int tachyon, int baryon, int pommel, int mb, u64 fuseid, int region, u8 *setparam);
int idsRegenerationGetIndex(IdsIndex *index, int *n);
int idsRegenerationGetHwConfigKeys(u8 *buf);
int idsRegenerationGetMGKeys(u8 *buf);
int idsRegenerationGetFactoryBadBlocksKey(u8 *buf);
int idsRegenerationGetSerialKey(u8 *buf);
int idsRegenerationGetWlanKey(u8 *buf);
int idsRegenerationGetAudioVolumeSetupKey(u8 *buf);
int idsRegenerationGetUsbKeys(u8 *buf);
int idsRegenerationGetUnkKey140(u8 *buf);
int idsRegenerationGetMGKey40(u8 *buf);
int idsRegenerationGetUnkKeys3X(u8 *buf);
int idsRegenerationGetParentalLockKey(u8 *buf);
int idsRegenerationGenerateFactoryFirmwareKey(u8 *buf);
int idsRegenerationGetLCDKey(u8 *buf);
int idsRegenerationGenerateCallibrationKey(u8 *buf);
int idsRegenerationGetUnkKeys5253(u8 *buf);
int idsRegenerationGetDefaultXMBColorKey(u8 *buf);
int idsRegenerationCreateCertificatesAndUMDKeys(u8 *buf);

#endif

