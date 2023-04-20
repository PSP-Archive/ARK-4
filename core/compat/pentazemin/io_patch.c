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

#include "common.h"
#include "io_patch.h"

PspIoDrv *ms_drv;
PspIoDrv *flashfat_drv;

PspIoDrvFuncs ms_funcs;
PspIoDrvFuncs flash_funcs;

int (* _sceIoAddDrv)(PspIoDrv *drv);
int (* _sceIoDelDrv)(const char *drv_name);

int (* _sceIoUnassign)(const char *dev);
int (* _sceIoAssign)(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2);

char *stristr(const char *str1, const char *str2) {
	#define MAXLEN 256

	static char temp1[MAXLEN+1], temp2[MAXLEN+1];
	temp1[MAXLEN] = 0;
	temp2[MAXLEN] = 0;

	strncpy(temp1, str1, MAXLEN);
	strncpy(temp2, str2, MAXLEN);

	int i;
	for (i = 0; i < MAXLEN && (temp1[i] != 0); i++) {
		temp1[i] = tolower((int)temp1[i]);
	}

	for (i = 0; i < MAXLEN && (temp2[i] != 0); i++) {
		temp2[i] = tolower((int)temp2[i]);
	}

	const char *pos = strstr(temp1, temp2);
	if (pos) {
		return (char *)(pos - temp1 + str1);
	}

	return 0;
}

int NoDevicePatch(u32 cmd) {
	if ((cmd & 0x00FFFF00) == 0x00208000) {
		// Returning error allows pops decryption?
		return ((cmd & 0xFF) == 0x82) ? -1 : 0;
	}

	return 1;
}

int _msIoOpen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *file = (char *)args[1];
	int flags = args[2];
	SceMode mode = (SceMode)args[3];

	int res = ms_funcs.IoOpen(arg, file, flags, mode);

	// Fixes Tekken and Soul Calibur
	if (stristr(file, "PARAM.SFO")) {
		((u32 *)arg)[18] = -1;
	}

	return res;
}

int _msIoIoctl(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	u32 cmd = (u32)args[1];
	void *indata = (void *)args[2];
	int inlen = args[3];
	void *outdata = (void *)args[4];
	int outlen = args[5];

	if (((u32 *)arg)[18] != -1) {
		int res = NoDevicePatch(cmd);
		if (res <= 0)
			return res;
	}

	return ms_funcs.IoIoctl(arg, cmd, indata, inlen, outdata, outlen);
}

int IoDevctlReinsertMs() {
	SceModule2 *mod = sceKernelFindModuleByName("sceKermitMsfs_driver");
	if (!mod)
		return -1;

	int (* MsfsSysEventHandler)(int ev_id, char *ev_name, void *param, int *result) = (void *)mod->text_addr + 0x150;

	// Perform a MS reinsertion
	static int ev_ids[] = { 0x102, 0x400, 0x10000, 0x100000 };

	int i;
	for (i = 0; i < (sizeof(ev_ids) / sizeof(int)); i++) {
		MsfsSysEventHandler(ev_ids[i], NULL, NULL, NULL);
	}

	return 0;
}

int _msIoDevctl(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *devname = (const char *)args[1];
	u32 cmd = (u32)args[2];
	void *indata = (void *)args[3];
	int inlen = args[4];
	void *outdata = (void *)args[5];
	int outlen = args[6];

	if (cmd == 0x02415830) {
		char *oldname = (char *)_lw((u32)indata);
		char *newname = (char *)_lw((u32)indata + 4);
		return ms_funcs.IoRename(arg, oldname, newname);
	} else if (cmd == 0x0240D81E) {
		return IoDevctlReinsertMs();
	} else if (cmd == 0x02425856) {
		// Character code set
		return 0;
	}

	return ms_funcs.IoDevctl(arg, devname, cmd, indata, inlen, outdata, outlen);
}

int msIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)flags;
	args[3] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_msIoOpen, args);
}

int msIoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	u32 args[6];
	args[0] = (u32)arg;
	args[1] = (u32)cmd;
	args[2] = (u32)indata;
	args[3] = (u32)inlen;
	args[4] = (u32)outdata;
	args[5] = (u32)outlen;

	return sceKernelExtendKernelStack(0x4000, (void *)_msIoIoctl, args);
}

int msIoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	u32 args[7];
	args[0] = (u32)arg;
	args[1] = (u32)devname;
	args[2] = (u32)cmd;
	args[3] = (u32)indata;
	args[4] = (u32)inlen;
	args[5] = (u32)outdata;
	args[6] = (u32)outlen;

	return sceKernelExtendKernelStack(0x4000, (void *)_msIoDevctl, args);
}

__attribute__((noinline)) int BuildMsPathChangeFsNum(PspIoDrvFileArg *arg, const char *name, char *ms_path) {
	sprintf(ms_path, "/__ADRENALINE__/flash%d%s", (int)arg->fs_num, name);

	int fs_num = arg->fs_num;
	arg->fs_num = 0;
	return fs_num;
}

int _flashIoOpen(u32 *args) {
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

int _flashIoClose(PspIoDrvFileArg *arg) {
	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoClose(arg);
	}

	return flash_funcs.IoClose(arg);
}

int _flashIoRead(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *data = (char *)args[1];
	int len = args[2];

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoRead(arg, data, len);
	}

	return flash_funcs.IoRead(arg, data, len);
}

int _flashIoWrite(u32 *args) {
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

SceOff _flashIoLseek(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	u32 ofs = (u32)args[1];
	int whence = args[2];

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoLseek(arg, ofs, whence);
	}

	return flash_funcs.IoLseek(arg, ofs, whence);
}

int _flashIoRemove(u32 *args) {
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

int _flashIoMkdir(u32 *args) {
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

int _flashIoRmdir(u32 *args) {
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

int _flashIoDopen(u32 *args) {
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

int _flashIoDclose(PspIoDrvFileArg *arg) {
	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoDclose(arg);
	}

	return flash_funcs.IoDclose(arg);
}

int _flashIoDread(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	SceIoDirent *dir = (SceIoDirent *)args[1];

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoDread(arg, dir);
	}

	return flash_funcs.IoDread(arg, dir);
}

int _flashIoGetstat(u32 *args) {
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

int _flashIoChstat(u32 *args) {
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

int _flashIoChdir(u32 *args) {
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

int sceIoAddDrvPatched(PspIoDrv *drv) {
	if (strcmp(drv->name, "ms") == 0) {
		ms_drv = drv;
		return 0;
	} else if (strcmp(drv->name, "flashfat") == 0) {
		flashfat_drv = drv;
		return 0;
	} else if (strcmp(drv->name, "fatms") == 0) {
		memcpy(&ms_funcs, drv->funcs, sizeof(PspIoDrvFuncs));

		drv->funcs->IoOpen = msIoOpen;
		drv->funcs->IoIoctl = msIoIoctl;
		drv->funcs->IoDevctl = msIoDevctl;

		// Add ms driver
		ms_drv->funcs = drv->funcs;
		_sceIoAddDrv(ms_drv);
	}
	else if (strcmp(drv->name, "flash") == 0) {
		memcpy(&flash_funcs, drv->funcs, sizeof(PspIoDrvFuncs));

		drv->funcs->IoOpen = flashIoOpen;
		drv->funcs->IoClose = flashIoClose;
		drv->funcs->IoRead = flashIoRead;
		drv->funcs->IoWrite = flashIoWrite;
		drv->funcs->IoLseek = flashIoLseek;
		drv->funcs->IoIoctl = flashIoIoctl;
		drv->funcs->IoRemove = flashIoRemove;
		drv->funcs->IoMkdir = flashIoMkdir;
		drv->funcs->IoRmdir = flashIoRmdir;
		drv->funcs->IoDopen = flashIoDopen;
		drv->funcs->IoDclose = flashIoDclose;
		drv->funcs->IoDread = flashIoDread;
		drv->funcs->IoGetstat = flashIoGetstat;
		drv->funcs->IoChstat = flashIoChstat;
		drv->funcs->IoChdir = flashIoChdir;
		drv->funcs->IoDevctl = flashIoDevctl;

		// Add flashfat driver
		flashfat_drv->funcs = drv->funcs;
		_sceIoAddDrv(flashfat_drv);
	}

	return _sceIoAddDrv(drv);
}

int sceIoDelDrvPatched(const char *drv_name) {
	if (strcmp(drv_name, "ms") == 0 || strcmp(drv_name, "flashfat") == 0) {
		return 0;
	} else if (strcmp(drv_name, "fatms") == 0) {
		_sceIoDelDrv("ms");
	} else if (strcmp(drv_name, "flash") == 0) {
		_sceIoDelDrv("flashfat");
	}

	return _sceIoDelDrv(drv_name);
}

int sceIoUnassignPatched(const char *dev) {
	int k1 = pspSdkSetK1(0);

	if (strncmp(dev, "ms", 2) == 0 || strncmp(dev, "flash", 5) == 0) {
		pspSdkSetK1(k1);
		return 0;
	}

	pspSdkSetK1(k1);
	return _sceIoUnassign(dev);
}

int sceIoAssignPatched(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2) {
	int k1 = pspSdkSetK1(0);

	if (strncmp(dev1, "ms", 2) == 0 || strncmp(dev1, "flash", 5) == 0) {
		pspSdkSetK1(k1);
		return 0;
	}

	pspSdkSetK1(k1);
	return _sceIoAssign(dev1, dev2, dev3, mode, unk1, unk2);
}

void PatchIoFileMgr() {

	u32 IoAddDrv = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x8E982A74);
	u32 IoDelDrv = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xC7F35804);
	u32 IoAssign = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xB2A628C1);
	u32 IoUnassign = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x6D08A871);

	HIJACK_FUNCTION(IoAddDrv, sceIoAddDrvPatched, _sceIoAddDrv);
	HIJACK_FUNCTION(IoDelDrv, sceIoDelDrvPatched, _sceIoDelDrv);

	// This fixes popsman flash2 assign
	HIJACK_FUNCTION(IoUnassign, sceIoUnassignPatched, _sceIoUnassign);
	HIJACK_FUNCTION(IoAssign, sceIoAssignPatched, _sceIoAssign);

	flushCache();
}