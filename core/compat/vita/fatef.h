#ifndef FATEF_H
#define FATEF_H

#include <pspsdk.h>
#include <pspkernel.h>

int sceIoEfOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode);
int sceIoEfRemoveHook(PspIoDrvFileArg * arg, char * file);
int sceIoEfMkdirHook(PspIoDrvFileArg * arg, char * file, SceMode mode);
int sceIoEfRmdirHook(PspIoDrvFileArg * arg, char * file);
int sceIoEfDopenHook(PspIoDrvFileArg * arg, char * file);
int sceIoEfGetStatHook(PspIoDrvFileArg * arg, char * file, SceIoStat* stat);
int sceIoEfChStatHook(PspIoDrvFileArg * arg, char * file, SceIoStat* stat, int bits);
int sceIoEfRenameHook(PspIoDrvFileArg *arg, const char *oldname, const char *newname);
int sceIoEfChdirHook(PspIoDrvFileArg *arg, const char *dir);

#endif