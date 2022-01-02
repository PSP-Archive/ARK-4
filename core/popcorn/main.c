/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>

#include "pspmodulemgr_kernel.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "globals.h"
#include "macros.h"

struct Hooks {
	u32 nid;
	void *fp;
};

SEConfig conf;

enum {
	ICON0_OK = 0,
	ICON0_MISSING = 1,
	ICON0_CORRUPTED = 2,
};

PSP_MODULE_INFO("PopcornManager", 0x1007, 1, 1);

#define PGD_ID "XX0000-XXXX00000_00-XXXXXXXXXX000XXX"
#define ACT_DAT "flash2:/act.dat"

#define RIF_MAGIC_FD 0x10000
#define ACT_DAT_FD 0x10001

extern u8 g_icon_png[0x3730];

static u32 g_icon0_status;
static u32 g_keys_bin_found;
static u32 g_is_custom_ps1;
static SceUID g_plain_doc_fd = -1;

static STMOD_HANDLER g_previous = NULL;

static u8 g_keys[16];

u32 psp_fw_version = 0;
u32 psp_model = 0;

static int myIoRead(int fd, u8 *buf, int size)
{
	int ret;
	u32 pos;
	u32 k1;

	k1 = pspSdkSetK1(0);

	if(fd != RIF_MAGIC_FD && fd != ACT_DAT_FD) {
		pos = sceIoLseek32(fd, 0, SEEK_CUR);
	} else {
		pos = 0;
	}
	
	if(g_keys_bin_found || g_is_custom_ps1) {
		if(fd == RIF_MAGIC_FD) {
			size = 152;
			printk("%s: fake rif content %d\n", __func__, size);
			memset(buf, 0, size);
			strcpy((char*)(buf+0x10), PGD_ID);
			ret = size;
			goto exit;
		} else if (fd == ACT_DAT_FD) {
			printk("%s: fake act.dat content %d\n", __func__, size);
			memset(buf, 0, size);
			ret = size;
			goto exit;
		}
	}
	
	ret = sceIoRead(fd, buf, size);

	if(ret != size) {
		goto exit;
	}

	if (size == 4) {
		u32 magic;

		magic = 0x464C457F; // ~ELF

		if(0 == memcmp(buf, &magic, sizeof(magic))) {
			magic = 0x5053507E; // ~PSP
			memcpy(buf, &magic, sizeof(magic));
			printk("%s: patch ~ELF -> ~PSP\n", __func__);
		}

		ret = size;
		goto exit;
	}
	
	if(size == sizeof(g_icon_png)) {
		u32 png_signature = 0x474E5089;

		if(g_icon0_status == ICON0_MISSING || ((g_icon0_status == ICON0_CORRUPTED) && 0 == memcmp(buf, &png_signature, 4))) {
			printk("%s: fakes a PNG for icon0\n", __func__);
			memcpy(buf, g_icon_png, size);

			ret = size;
			goto exit;
		}
	}

	if (g_is_custom_ps1 && size >= 0x420 && buf[0x41B] == 0x27 &&
			buf[0x41C] == 0x19 &&
			buf[0x41D] == 0x22 &&
			buf[0x41E] == 0x41 &&
			buf[0x41A] == buf[0x41F]) {
		buf[0x41B] = 0x55;
		printk("%s: unknown patch loc_6c\n", __func__);
	}

exit:
	pspSdkSetK1(k1);
	printk("%s: fd=0x%08X pos=0x%08X size=%d -> 0x%08X\n", __func__, (uint)fd, (uint)pos, (int)size, ret);

	return ret;
}

static int myIoReadAsync(int fd, u8 *buf, int size)
{
	int ret;
	u32 pos;
	u32 k1;

	k1 = pspSdkSetK1(0);
	pos = sceIoLseek32(fd, 0, SEEK_CUR);
	ret = sceIoReadAsync(fd, buf, size);
	printk("%s: 0x%08X 0x%08X 0x%08X -> 0x%08X\n", __func__, (uint)fd, (uint)pos, size, ret);
	pspSdkSetK1(k1);

	return ret;
}

static SceOff myIoLseek(SceUID fd, SceOff offset, int whence)
{
	SceOff ret;
	u32 k1;

	k1 = pspSdkSetK1(0);

	if(g_keys_bin_found || g_is_custom_ps1) {
		if (fd == RIF_MAGIC_FD) {
			printk("%s: [FAKE]\n", __func__);
			ret = 0;
		} else if (fd == ACT_DAT_FD) {
			printk("%s: [FAKE]\n", __func__);
			ret = 0;
		} else {
			ret = sceIoLseek(fd, offset, whence);
		}
	} else {
		ret = sceIoLseek(fd, offset, whence);
	}

	pspSdkSetK1(k1);
	printk("%s: 0x%08X 0x%08X 0x%08X -> 0x%08X\n", __func__, (uint)fd, (uint)offset, (uint)whence, (int)ret);

	return ret;
}

static int myIoClose(SceUID fd)
{
	int ret;
	u32 k1;

	k1 = pspSdkSetK1(0);

	if(g_keys_bin_found || g_is_custom_ps1) {
		if (fd == RIF_MAGIC_FD) {
			printk("%s: [FAKE]\n", __func__);
			ret = 0;
		} else if (fd == ACT_DAT_FD) {
			printk("%s: [FAKE]\n", __func__);
			ret = 0;
		} else {
			ret = sceIoClose(fd);
		}
	} else {
		ret = sceIoClose(fd);
	}

	if(g_plain_doc_fd == fd && ret == 0) {
		g_plain_doc_fd = -1;
	}

	pspSdkSetK1(k1);
	printk("%s: 0x%08X -> 0x%08X\n", __func__, fd, ret);

	return ret;
}

static const char *get_filename(const char *path)
{
	const char *p;

	if(path == NULL) {
		return NULL;
	}

	p = strrchr(path, '/');

	if(p == NULL) {
		p = path;
	} else {
		p++;
	}

	return p;
}

static int is_eboot_pbp_path(const char *path)
{
	const char *p;

	p = get_filename(path);

	if(p != NULL && 0 == strcmp(p, "EBOOT.PBP")) {
		return 1;
	}

	return 0;
}

static int check_file_is_decrypted(const char *filename)
{
	SceUID fd;
	u32 k1;
	int result = 0, ret;
	u8 p[16 + 64], *buf;
	u32 *magic;

	buf = (u8*)((((u32)p) & ~(64-1)) + 64);

	if(!g_is_custom_ps1 && is_eboot_pbp_path(filename)) {
		return 0;
	}

	k1 = pspSdkSetK1(0);
	fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);

	if(fd < 0) {
		goto exit;
	}

	ret = sceIoRead(fd, buf, 16);

	if(ret != 16) {
		goto exit;
	}

	magic = (u32*)buf;

	if(*magic == 0x44475000) { // PGD
		goto exit;
	}

	result = 1;

exit:
	if(fd >= 0) {
		sceIoClose(fd);
	}

	pspSdkSetK1(k1);

	return result;
}

static int is_document_path(const char *path)
{
	const char *p;

	p = get_filename(path);
	
	if(p != NULL && 0 == strcmp(p, "DOCUMENT.DAT")) {
		return 1;
	}

	return 0;
}

static int sceIoOpenPlain(const char *file, int flag, int mode)
{
	int ret;

	if(flag == 0x40000001 && check_file_is_decrypted(file)) {
		printk("%s: removed PGD open flag\n", __func__);
		ret = sceIoOpen(file, flag & ~0x40000000, mode);

		if(ret >= 0 && is_document_path(file)) {
			g_plain_doc_fd = ret;
		}
	} else {
		ret = sceIoOpen(file, flag, mode);
	}

	return ret;
}

static int myIoOpen(const char *file, int flag, int mode)
{
	int ret;

	if(g_keys_bin_found || g_is_custom_ps1) {
		if(strstr(file, PGD_ID)) {
			printk("%s: [FAKE]\n", __func__);
			ret = RIF_MAGIC_FD;
		} else if (0 == strcmp(file, ACT_DAT)) {
			printk("%s: [FAKE]\n", __func__);
			ret = ACT_DAT_FD;
		} else {
			ret = sceIoOpenPlain(file, flag, mode);
		}		
	} else {
		ret = sceIoOpenPlain(file, flag, mode);
	}

	printk("%s: %s 0x%08X -> 0x%08X\n", __func__, file, flag, ret);

	return ret;
}

static int myIoIoctl(SceUID fd, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen)
{
	int ret;

	if(cmd == 0x04100001) {
		printk("%s: setting PGD key\n", __func__);
		//hexdump(indata, inlen);
	}

	if(cmd == 0x04100002) {
		printk("%s: setting PGD offset: 0x%08X\n", __func__, *(uint*)indata);
	}

	if (g_is_custom_ps1 || (g_plain_doc_fd >= 0 && g_plain_doc_fd == fd)) {
		if (cmd == 0x04100001) {
			ret = 0;
			printk("%s: [FAKE] 0x%08X 0x%08X -> 0x%08X\n", __func__, fd, cmd, ret);
			goto exit;
		}

		if (cmd == 0x04100002) {
			ret = sceIoLseek32(fd, *(u32*)indata, PSP_SEEK_SET);

			if(ret < 0) {
				printk("%s: sceIoLseek32 -> 0x%08X\n", __func__, ret);
			}

			ret = 0;
			printk("%s: [FAKE] 0x%08X 0x%08X -> 0x%08X\n", __func__, fd, cmd, ret);
			goto exit;
		}
	}

	ret = sceIoIoctl(fd, cmd, indata, inlen, outdata, outlen);

exit:
	printk("%s: 0x%08X -> 0x%08X\n", __func__, fd, ret);

	return ret;
}

static int myIoGetstat(const char *path, SceIoStat *stat)
{
	int ret;

	if(g_keys_bin_found || g_is_custom_ps1) {
		if(strstr(path, PGD_ID)) {
			stat->st_mode = 0x21FF;
			stat->st_attr = 0x20;
			stat->st_size = 152;
			ret = 0;
			printk("%s: [FAKE]\n", __func__);
		} else if (0 == strcmp(path, ACT_DAT)) {
			stat->st_mode = 0x21FF;
			stat->st_attr = 0x20;
			stat->st_size = 4152;
			ret = 0;
			printk("%s: [FAKE]\n", __func__);
		} else {
			ret = sceIoGetstat(path, stat);
		}
	} else {
		ret = sceIoGetstat(path, stat);
	}

	printk("%s: %s -> 0x%08X\n", __func__, path, ret);

	return ret;
}

static int (*_get_rif_path)(const char *name, char *path) = NULL;

static int get_rif_path(char *name, char *path)
{
	int ret;

	if(g_keys_bin_found || g_is_custom_ps1) {
		strcpy(name, PGD_ID);
	}

	ret = (*_get_rif_path)(name, path);
	printk("%s: %s %s -> 0x%08X\n", __func__, name, path, ret);

	return ret;
}

static int get_keypath(char *keypath, int size)
{
	char *p;

	strncpy(keypath, size, sceKernelInitFileName());
	p = strrchr(keypath, '/');

	if(p == NULL) {
		return -1;
	}

	p[1] = '\0';
	strncat(keypath, size, "KEYS.BIN");

	return 0;
}

static int save_key(const char *keypath, u8 *key, int size)
{
	SceUID keys;
	int ret;

	keys = sceIoOpen(keypath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (keys < 0) {
		return -1;
	}

	ret = sceIoWrite(keys, key, size);

	if(ret == size) {
		ret = 0;
	} else {
		ret = -2;
	}

	sceIoClose(keys);

	return ret;
}

static int load_key(const char *keypath, u8 *key, int size)
{
	SceUID keys; 
	int ret;

	keys = sceIoOpen(keypath, PSP_O_RDONLY, 0777);

	if (keys < 0) {
		printk("%s: sceIoOpen %s -> 0x%08X\n", __func__, keypath, keys);

		return -1;
	}

	ret = sceIoRead(keys, key, size); 

	if (ret == size) {
		ret = 0;
	} else {
		ret = -2;
	}

	sceIoClose(keys);

	return ret;
}

static int (*sceNpDrmGetVersionKey)(u8 * key, u8 * act, u8 * rif, u32 flags);

static int _sceNpDrmGetVersionKey(u8 * key, u8 * act, u8 * rif, u32 flags)
{
	char keypath[128];
	int ret, result;
   
	result = (*sceNpDrmGetVersionKey)(key, act, rif, flags);

	if (g_is_custom_ps1) {
		printk("%s: -> 0x%08X\n", __func__, result);
		result = 0;

		if (g_keys_bin_found) {
			memcpy(key, g_keys, sizeof(g_keys));
		}
		
		printk("%s:[FAKE] -> 0x%08X\n", __func__, result);
	} else {
		get_keypath(keypath, sizeof(keypath));

		if (result == 0) {
			memcpy(g_keys, key, sizeof(g_keys));
			ret = save_key(keypath, g_keys, sizeof(g_keys));
			printk("%s: save_key -> %d\n", __func__, ret);
		} else {
			if (g_keys_bin_found) {
				memcpy(key, g_keys, sizeof(g_keys));
				result = 0;
			}
		}
	}
	
	return result;
}

static int (*scePspNpDrm_driver_9A34AC9F)(u8 *rif);

static int _scePspNpDrm_driver_9A34AC9F(u8 *rif)
{
	int result;

	result = (*scePspNpDrm_driver_9A34AC9F)(rif);
	printk("%s: 0x%08X -> 0x%08X\n", __func__, (uint)rif, result);

	if (result != 0) {
		if (g_keys_bin_found || g_is_custom_ps1) {
			result = 0;
			printk("%s:[FAKE] -> 0x%08X\n", __func__, result);
		}
	}

	return result;
}

static int _sceDrmBBCipherUpdate(void *ckey, u8 *data, int size)
{
	return 0;
}

static int _sceDrmBBCipherInit(void *ckey, int type, int mode, u8 *header_key, u8 *version_key, u32 seed)
{
	return 0;
}

static int _sceDrmBBMacInit(void *mkey, int type)
{
	return 0;
}

static int _sceDrmBBMacUpdate(void *mkey, u8 *buf, int size)
{
	return 0;
}

static int _sceDrmBBCipherFinal(void *ckey)
{
	return 0;
}

static int _sceDrmBBMacFinal(void *mkey, u8 *buf, u8 *vkey)
{
	return 0;
}

static int _sceDrmBBMacFinal2(void *mkey, u8 *out, u8 *vkey)
{
	return 0;
}

static struct Hooks g_io_hooks[] = {
	{ 0x109F50BC, &myIoOpen, },
	{ 0x27EB27B8, &myIoLseek, },
	{ 0x63632449, &myIoIoctl, },
	{ 0x6A638D83, &myIoRead, },
	{ 0xA0B5A7C2, &myIoReadAsync, },
	{ 0xACE946E8, &myIoGetstat, },
	{ 0x810C4BC3, &myIoClose, },
};

static struct Hooks g_amctrl_hooks[] = {
	{ 0x1CCB66D2, &_sceDrmBBCipherInit, },
	{ 0x0785C974, &_sceDrmBBCipherUpdate, },
	{ 0x9951C50F, &_sceDrmBBCipherFinal, },
	{ 0x525B8218, &_sceDrmBBMacInit, },
	{ 0x58163FBE, &_sceDrmBBMacUpdate, },
	{ 0xEF95A213, &_sceDrmBBMacFinal, },
	{ 0xF5186D8E, &_sceDrmBBMacFinal2, },
};

void patchPopsMgr(void)
{
    SceModule2 *mod = (SceModule2*) sceKernelFindModuleByName("scePops_Manager");
    unsigned int text_addr = mod->text_addr;
    int i;
    
    sceNpDrmGetVersionKey = (void*)sctrlHENFindFunction("scePspNpDrm_Driver", "scePspNpDrm_driver", 0x0F9547E6);
    scePspNpDrm_driver_9A34AC9F = (void*)sctrlHENFindFunction("scePspNpDrm_Driver", "scePspNpDrm_driver", 0x9A34AC9F);

    for(i=0; i<NELEMS(g_io_hooks); ++i)
    {
        hookImportByNID(mod, "IoFileMgrForKernel", g_io_hooks[i].nid, g_io_hooks[i].fp);
    }
    // find getRifPath
    for (u32 addr = text_addr; addr<text_addr+mod->text_size && _get_rif_path==NULL; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x2C830001 && _lw(addr+4) == 0x2CA70001){
            _get_rif_path = (void*)(addr-4);
            break;
        }
    }
    // patch popsman
    int patches = 5;
    for (u32 addr = text_addr; addr<text_addr+mod->text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(_get_rif_path)){
            _sw(JAL(&get_rif_path), addr);
            patches--;
        }
        else if (data == 0x0000000D){
            _sw(NOP, addr); // remove the check in scePopsManLoadModule that only allows loading module below the FW 3.XX
            patches--;
        }
        else if (data == JAL(scePspNpDrm_driver_9A34AC9F)){
            _sw(JAL(_scePspNpDrm_driver_9A34AC9F), addr); // hook scePspNpDrm_driver_9A34AC9F call
            patches--;
        }
        else if (data == JUMP(sceNpDrmGetVersionKey)){
            _sw(JUMP(_sceNpDrmGetVersionKey), addr); // hook sceNpDrmGetVersionKey call
            patches--;
        }
    }
    if (g_is_custom_ps1)
    {
        for(i=0; i<NELEMS(g_amctrl_hooks); ++i)
        {
            hookImportByNID(mod, "sceAmctrl_driver", g_amctrl_hooks[i].nid, g_amctrl_hooks[i].fp);
        }
    }

}

static u32 is_custom_ps1(void)
{
	SceUID fd = -1;
	const char *filename;
	int result, ret;
	u32 psar_offset, pgd_offset, *magic;
	u8 p[40 + 64], *header;

	header = (u8*)((((u32)p) & ~(64-1)) + 64);
	filename = sceKernelInitFileName();
	result = 0;

	if(filename == NULL) {
		result = 0;
		goto exit;
	}

	fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);

	if(fd < 0) {
		printk("%s: sceIoOpen %s -> 0x%08X\n", __func__, filename, fd);
		result = 0;
		goto exit;
	}

	ret = sceIoRead(fd, header, 40);

	if(ret != 40) {
		printk("%s: sceIoRead -> 0x%08X\n", __func__, ret);
		result = 0;
		goto exit;
	}

	psar_offset = *(u32*)(header+0x24);
	sceIoLseek32(fd, psar_offset, PSP_SEEK_SET);
	ret = sceIoRead(fd, header, 40);

	if(ret != 40) {
		printk("%s: sceIoRead -> 0x%08X\n", __func__, ret);
		result = 0;
		goto exit;
	}

	pgd_offset = psar_offset;

	if(0 == memcmp(header, "PSTITLE", sizeof("PSTITLE")-1)) {
		pgd_offset += 0x200;
	} else {
		pgd_offset += 0x400;
	}

	sceIoLseek32(fd, pgd_offset, PSP_SEEK_SET);
	ret = sceIoRead(fd, header, 4);

	if(ret != 4) {
		printk("%s: sceIoRead -> 0x%08X\n", __func__, ret);
		result = 0;
		goto exit;
	}

	magic = (u32*)header;

	// PGD offset
	if(*magic != 0x44475000) {
		printk("%s: custom pops found\n", __func__);
		result = 1;
	}

exit:
	if(fd >= 0) {
		sceIoClose(fd);
	}

	return result;
}

static int place_syscall_stub(void* func, void *addr)
{
	u32 syscall_num;
	extern u32 sceKernelQuerySystemCall(void *func);

	syscall_num = sceKernelQuerySystemCall(func);

	if(syscall_num == (u32)-1) {
		return -1;
	}

	_sw(0x03E00008, (u32)addr);
	_sw(((syscall_num<<6)|12), (u32)(addr+4));

	return 0;
}

static void reboot_vsh_with_error(u32 error)
{
	struct SceKernelLoadExecVSHParam param;	
	u32 vshmain_args[0x20/4];

	memset(&param, 0, sizeof(param));
	memset(vshmain_args, 0, sizeof(vshmain_args));

	vshmain_args[0/4] = 0x0400;
	vshmain_args[4/4] = 0x20;
	vshmain_args[0x14/4] = error;

	param.size = sizeof(param);
	param.args = 0x400;
	param.argp = vshmain_args;
	param.vshmain_args_size = 0x400;
	param.vshmain_args = vshmain_args;
	param.configfile = "/kd/pspbtcnf.txt";

	sctrlKernelExitVSH(&param);
}

int decompress_data(u32 destSize, const u8 *src, u8 *dest)
{
	u32 k1;
	int ret;

	k1 = pspSdkSetK1(0);

	if (destSize < 0) {
		reboot_vsh_with_error((u32)destSize);
		pspSdkSetK1(k1);

		return 0;
	}

	ret = sceKernelDeflateDecompress(dest, destSize, src, 0);
	printk("%s: 0x%08X 0x%08X 0x%08X -> 0x%08X\n", __func__, (uint)destSize, (uint)src, (uint)dest, ret);

	if (ret >= 0) {
		ret = 0x92FF;
		printk("%s: [FAKE] -> 0x%08X\n", __func__, ret);
	}

	pspSdkSetK1(k1);

	return ret;
}

static int patch_decompress_data(void *stub_addr, void *patch_addr)
{
	int ret;

	ret = place_syscall_stub(decompress_data, stub_addr);

	if (ret != 0) {
		printk("%s: place_syscall_stub -> 0x%08X\n", __func__, ret);

		return -1;
	}

	_sw(JAL(stub_addr), (u32)patch_addr);

	return 0;
}

static int (*sceMeAudio_67CD7972)(void *buf, int size);

int _sceMeAudio_67CD7972(void *buf, int size)
{
	int ret;
	u32 k1;

	k1 = pspSdkSetK1(0);
	ret = (*sceMeAudio_67CD7972)(buf, size);
	pspSdkSetK1(k1);

	printk("%s: 0x%08X -> 0x%08X\n", __func__, size, ret);

	return ret;
}

static void patchPops(SceModule2 *mod)
{
    unsigned int text_addr = mod->text_addr;
    void *stub_addr=NULL, *patch_addr=NULL;
    printk("%s: patching pops\r\n", __func__);

    for (u32 addr = text_addr; addr<text_addr+mod->text_size; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x8E66000C)
            patch_addr = (void*)(addr+8);
        else if (data == 0x3C1D09BF)
            stub_addr = (void*)(U_EXTRACT_CALL(addr-16));
        else if (data == 0x00432823 && g_icon0_status != ICON0_OK)
            _sw(0x24050000 | (sizeof(g_icon_png) & 0xFFFF), addr); // patch icon0 size
        else if (data == 0x24050080 && _lw(addr+24) == 0x24030001)
            _sw(0x24020001, addr+8); // Patch Manual Name Check
    }

    if(g_is_custom_ps1){
        patch_decompress_data(stub_addr, patch_addr);
    }
    
    // Prevent Permission Problems
    sceMeAudio_67CD7972 = (void*)sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x2AB4FE43);
    hookImportByNID(mod, "sceMeAudio", 0x2AB4FE43, _sceMeAudio_67CD7972);

}

static void popcorn_patch_chain(SceModule2 *mod)
{
	printk("%s: %s\n", __func__, mod->modname);

	if (0 == strcmp(mod->modname, "pops")) {
		patchPops(mod);
		flushCache();
	}

	if(g_previous)
		g_previous(mod);
}

static int get_icon0_status(void)
{
	u32 icon0_offset = 0;
	int result = ICON0_MISSING;
	SceUID fd = -1;;
	const char *filename;
	u8 p[40 + 64], *header;
	
	header = (u8*)((((u32)p) & ~(64-1)) + 64);
	filename = sceKernelInitFileName();

	if(filename == NULL) {
		goto exit;
	}
	
	fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);

	if(fd < 0) {
		printk("%s: sceIoOpen %s -> 0x%08X\n", __func__, filename, fd);
		goto exit;
	}
	
	sceIoRead(fd, header, 40);
	icon0_offset = *(u32*)(header+0x0c);
	sceIoLseek32(fd, icon0_offset, PSP_SEEK_SET);
	sceIoRead(fd, header, 40);

	if(*(u32*)(header+4) == 0xA1A0A0D) {
		if ( *(u32*)(header+0xc) == 0x52444849 && // IHDR
				*(u32*)(header+0x10) == 0x50000000 && // 
				*(u32*)(header+0x14) == *(u32*)(header+0x10)
		   ) {
			result = ICON0_OK;
		} else {
			result = ICON0_CORRUPTED;
		}
	} else {
		result = ICON0_MISSING;
	}

	printk("%s: PNG file status -> %d\n", __func__, result);

exit:
	if(fd >= 0) {
		sceIoClose(fd);
	}

	return result;
}

static void setup_psx_fw_version(u32 fw_version)
{
	int (*_SysMemUserForUser_315AD3A0)(u32 fw_version);
	
	_SysMemUserForUser_315AD3A0 = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", 0x315AD3A0);

	if (_SysMemUserForUser_315AD3A0 == NULL) {
		printk("_SysMemUserForUser_315AD3A0 not found\n");
		reboot_vsh_with_error(0x80000001);
	}

	_SysMemUserForUser_315AD3A0(fw_version);
}

int module_start(SceSize args, void* argp)
{
	char keypath[128];
	int ret;
	SceIoStat stat;

	psp_fw_version = sceKernelDevkitVersion();
	psp_model = sceKernelGetModel();
	memset(&conf, 0, sizeof(conf));
	sctrlSEGetConfig(&conf);
	printk("Popcorn: init_file = %s psp_fw_version = 0x%08X psp_model = %d\n", sceKernelInitFileName(), (uint)psp_fw_version, (int)psp_model);

	get_keypath(keypath, sizeof(keypath));
	ret = sceIoGetstat(keypath, &stat);
	g_keys_bin_found = 0;

	if(ret == 0) {
		ret = load_key(keypath, g_keys, sizeof(g_keys));

		if(ret == 0) {
			g_keys_bin_found = 1;
			printk("keys.bin found\n");
		}
	}

	g_is_custom_ps1 = is_custom_ps1();
	g_icon0_status = get_icon0_status();

	if(g_is_custom_ps1) {
		setup_psx_fw_version(psp_fw_version);
	}

	g_previous = sctrlHENSetStartModuleHandler(&popcorn_patch_chain);
	patchPopsMgr();
	flushCache();
	
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}
