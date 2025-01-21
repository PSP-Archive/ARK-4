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

#include "flashfs.h"

extern PspIoDrvFuncs ms_funcs;
extern PspIoDrvFuncs flash_funcs;

extern __attribute__((noinline)) int BuildMsPathChangeFsNum(PspIoDrvFileArg *arg, const char *name, char *ms_path);

static int _flashIoOpen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *file = (char *)args[1];
	int flags = args[2];
	SceMode mode = (SceMode)args[3];

	if (strcmp(file, "/kd/mpeg_vsh.prx") == 0) {
		file = "/kd/mpeg.prx";
	}

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);
	int res = ms_funcs.IoOpen(arg, ms_path, flags, mode);
	if (res >= 0) {
		((u32 *)arg)[18] = 1;
	}

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;

	return flash_funcs.IoOpen(arg, file, flags, mode);	
}

static int _flashIoClose(PspIoDrvFileArg *arg) {
	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoClose(arg);
	}

	return flash_funcs.IoClose(arg);
}

static int _flashIoRead(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *data = (char *)args[1];
	int len = args[2];

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoRead(arg, data, len);
	}

	return flash_funcs.IoRead(arg, data, len);
}

static int _flashIoWrite(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *data = (const char *)args[1];
	int len = args[2];

	if (!data && len == 0) {
		return 0;
	}

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoWrite(arg, data, len);
	}

	return flash_funcs.IoWrite(arg, data, len);
}

static SceOff _flashIoLseek(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	u32 ofs = (u32)args[1];
	int whence = args[2];

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoLseek(arg, ofs, whence);
	}

	return flash_funcs.IoLseek(arg, ofs, whence);
}

static int _flashIoRemove(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = ms_funcs.IoRemove(arg, ms_path);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoRemove(arg, name);
}

static int _flashIoMkdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];
	SceMode mode = (SceMode)args[2];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = ms_funcs.IoMkdir(arg, ms_path, mode);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoMkdir(arg, name, mode);
}

static int _flashIoRmdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = ms_funcs.IoRmdir(arg, ms_path);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoRmdir(arg, name);
}

static int _flashIoDopen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *dirname = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, dirname, ms_path);
	int res = ms_funcs.IoDopen(arg, ms_path);
	if (res >= 0) {
		((u32 *)arg)[18] = 1;
	}

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoDopen(arg, dirname);
}

static int _flashIoDclose(PspIoDrvFileArg *arg) {
	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoDclose(arg);
	}

	return flash_funcs.IoDclose(arg);
}

static int _flashIoDread(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	SceIoDirent *dir = (SceIoDirent *)args[1];

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoDread(arg, dir);
	}

	return flash_funcs.IoDread(arg, dir);
}

static int _flashIoGetstat(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);
	int res = ms_funcs.IoGetstat(arg, ms_path, stat);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoGetstat(arg, file, stat);
}

static int _flashIoChstat(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];
	int bits = (int)args[3];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);
	int res = ms_funcs.IoChstat(arg, ms_path, stat, bits);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoChstat(arg, file, stat, bits);
}

static int _flashIoChdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *dir = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, dir, ms_path);
	int res = ms_funcs.IoChdir(arg, ms_path);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoChdir(arg, dir);
}

int NoDevicePatch(u32 cmd) {
	if ((cmd & 0x00FFFF00) == 0x00208000) {
		// Returning error allows pops decryption?
		return ((cmd & 0xFF) == 0x82) ? -1 : 0;
	}

	return 1;
}

int flashIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)flags;
	args[3] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoOpen, args);
}

int flashIoClose(PspIoDrvFileArg *arg) {
	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoClose, arg);
}

int flashIoRead(PspIoDrvFileArg *arg, char *data, int len) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)data;
	args[2] = (u32)len;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRead, args);
}

int flashIoWrite(PspIoDrvFileArg *arg, const char *data, int len) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)data;
	args[2] = (u32)len;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoWrite, args);
}

SceOff flashIoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)ofs;
	args[2] = (u32)whence;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoLseek, args);
}

int flashIoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	int res = NoDevicePatch(cmd);
	if (res <= 0)
		return res;

	return 0;
}

int flashIoRemove(PspIoDrvFileArg *arg, const char *name) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)name;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRemove, args);
}

int flashIoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)name;
	args[2] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoMkdir, args);
}

int flashIoRmdir(PspIoDrvFileArg *arg, const char *name) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)name;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRmdir, args);
}

int flashIoDopen(PspIoDrvFileArg *arg, const char *dirname) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dirname;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDopen, args);
}

int flashIoDclose(PspIoDrvFileArg *arg) {
	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDclose, arg);
}

int flashIoDread(PspIoDrvFileArg *arg, SceIoDirent *dir) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dir;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDread, args);
}

int flashIoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoGetstat, args);
}

int flashIoChstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;
	args[3] = (u32)bits;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoChstat, args);
}

int flashIoChdir(PspIoDrvFileArg *arg, const char *dir) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dir;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoChdir, args);
}

int flashIoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	return 0;
}