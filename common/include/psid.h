#ifndef PSID_H
#define PSID_H

#define ENCRYPTED_TAG_MAGIC_1 "\x5D\xB1\x1D\xC0"
#define ENCRYPTED_TAG_MAGIC_2 "\xD1\x51\xB9\x56"
#define PSID_SALT_MAGIC 0xC01DB15D

void prxXorKeyMix(unsigned char *dstBuf, unsigned int size, unsigned char *srcBuf, unsigned char *xorKey);

int isPrxEncrypted(unsigned char *prx, unsigned int size);

// for external modules only
void psidCheck(void);

#endif
