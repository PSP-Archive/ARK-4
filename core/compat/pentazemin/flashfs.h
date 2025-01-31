/*
    Adrenaline
    Copyright (C) 2016-2018, TheFloW

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FLASH_FS
#define FLASH_FS

#include <pspsdk.h>
#include <pspkernel.h>

int flashIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
int flashIoClose(PspIoDrvFileArg *arg);
int flashIoRead(PspIoDrvFileArg *arg, char *data, int len);
int flashIoWrite(PspIoDrvFileArg *arg, const char *data, int len);
SceOff flashIoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence);
int flashIoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
int flashIoRemove(PspIoDrvFileArg *arg, const char *name);
int flashIoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode);
int flashIoRmdir(PspIoDrvFileArg *arg, const char *name);
int flashIoDopen(PspIoDrvFileArg *arg, const char *dirname);
int flashIoDclose(PspIoDrvFileArg *arg);
int flashIoDread(PspIoDrvFileArg *arg, SceIoDirent *dir);
int flashIoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);
int flashIoChstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits);
int flashIoChdir(PspIoDrvFileArg *arg, const char *dir);
int flashIoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);

#endif