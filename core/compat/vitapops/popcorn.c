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

#include <pspsdk.h>
#include <globals.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <pspinit.h>

typedef struct {
	u32 magic;
	u32 version;
	u32 param_offset;
	u32 icon0_offset;
	u32 icon1_offset;
	u32 pic0_offset;
	u32 pic1_offset;
	u32 snd0_offset;
	u32 elf_offset;
	u32 psar_offset;
} PBPHeader;

#define ELF_MAGIC 0x464C457F
#define PSP_MAGIC 0x5053507E

#define KIRK7_HEADER_SIZE 0x14

#define VERSION_KEY_SIZE 0x10
#define CONTENT_ID_SIZE 0x30

typedef struct {
	int type;
	u8 key[16];
	u8 pad[16];
	int pad_size;
} MAC_KEY;

int sceDrmBBMacInit(MAC_KEY *mkey, int type){
    static void* ptr = NULL;
    if (!ptr) ptr = sctrlHENFindFunction("sceAmctrl_driver", "sceAmctrl_driver", 0x525B8218);
    int (*func)(MAC_KEY*, int) = ptr;
    return func(mkey, type);
}

int sceDrmBBMacUpdate(MAC_KEY *mkey, u8 *buf, int size){
    static void* ptr = NULL;
    if (!ptr) ptr = sctrlHENFindFunction("sceAmctrl_driver", "sceAmctrl_driver", 0x58163FBE);
    int (*func)(MAC_KEY*, u8*, int) = ptr;
    return func(mkey, buf, size);
}

int sceDrmBBMacFinal(MAC_KEY *mkey, u8 *buf, u8 *vkey){
    static void* ptr = NULL;
    if (!ptr) ptr = sctrlHENFindFunction("sceAmctrl_driver", "sceAmctrl_driver", 0xEF95A213);
    int (*func)(MAC_KEY*, u8*, u8*) = ptr;
    return func(mkey, buf, vkey);
}

int (* scePopsManExitVSHKernel)(int error);
int (* SetVersionKeyContentId)(char *file, u8 *version_key, char *content_id);

u8 pgd_buf[0x80];
int original = 0;


void ClearCaches() {
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

int scePopsManExitVSHKernelPatched(u32 destSize, u8 *src, u8 *dest) {
	if (destSize & 0x80000000)
		return scePopsManExitVSHKernel(destSize);

	int size = sceKernelDeflateDecompress(dest, destSize, src, 0);
	return (size == 0x9300) ? 0x92FF : size;
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
	// Remove drm flag
	return sceIoOpen(file, flags & ~0x40000000, mode);
}

int sceIoIoctlPatched(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	// Seek
	if (cmd == 0x04100002)
		sceIoLseek(fd, *(u32 *)indata, PSP_SEEK_SET);

	return 0;
}

int sceIoReadPatched(SceUID fd, void *data, SceSize size) {
	int res = sceIoRead(fd, data, size);

	// Fake ~PSP magic to avoid crash
	if (size == sizeof(u32)) {
		u32 magic = ELF_MAGIC;
		if (memcmp(data, &magic, sizeof(u32)) == 0) {
			magic = PSP_MAGIC;
			memcpy(data, &magic, sizeof(u32));
		}
	}

	return res;
}

// PGD decryption by Hykem
// https://github.com/Hykem/psxtract/blob/master/Linux/crypto.c

int kirk7(u8 *buf, int size, int type) {
	u32 *header = (u32 *)buf;

	header[0] = 5;
	header[1] = 0;
	header[2] = 0;
	header[3] = type;
	header[4] = size;

	return sceUtilsBufferCopyWithRange(buf, size + KIRK7_HEADER_SIZE, buf, size, 7);
}

int GetVersionKeyContentIdPatched(char *file, u8 *version_key, char *content_id) {
	u8 dummy_version_key[VERSION_KEY_SIZE];
	char dummy_content_id[CONTENT_ID_SIZE];

	if (!version_key)
		version_key = dummy_version_key;

	if (!content_id)
		content_id = dummy_content_id;

	memset(version_key, 0, VERSION_KEY_SIZE);
	memset(content_id, 0, CONTENT_ID_SIZE);

	if (original) {
		// Set mac type
		int mac_type = 0;

		if (((u32 *)pgd_buf)[2] == 1) {
			mac_type = 1;

			if (((u32 *)pgd_buf)[1] > 1)
				mac_type = 3;
		} else {
			mac_type = 2;
		}

		// Generate the key from MAC 0x70 
		MAC_KEY mac_key;
		sceDrmBBMacInit(&mac_key, mac_type);
		sceDrmBBMacUpdate(&mac_key, pgd_buf, 0x70);

		u8 xor_keys[VERSION_KEY_SIZE];
		sceDrmBBMacFinal(&mac_key, xor_keys, NULL);

		u8 kirk_buf[VERSION_KEY_SIZE + KIRK7_HEADER_SIZE];

		if (mac_key.type == 3) {
			memcpy(kirk_buf + KIRK7_HEADER_SIZE, pgd_buf + 0x70, VERSION_KEY_SIZE);
			kirk7(kirk_buf, VERSION_KEY_SIZE, 0x63);
		} else {
			memcpy(kirk_buf, pgd_buf + 0x70, VERSION_KEY_SIZE);
		}

		memcpy(kirk_buf + KIRK7_HEADER_SIZE, kirk_buf, VERSION_KEY_SIZE);
		kirk7(kirk_buf, VERSION_KEY_SIZE, (mac_key.type == 2) ? 0x3A : 0x38);

		// Get version key
		int i;
		for (i = 0; i < VERSION_KEY_SIZE; i++) {
			version_key[i] = xor_keys[i] ^ kirk_buf[i];
		}
	}

	return SetVersionKeyContentId(file, version_key, content_id);
}

int patchPops(SceModule2* mod){
    u32 i;
    for (i = 0; i < mod->text_size; i += 4) {
        if ((_lw(mod->text_addr+i) & 0xFFE0FFFF) == 0x3C0049FE) {
            _sh(0x4BCD, mod->text_addr+i);
        }
    }

    if (!original) {
        // Use our decompression function
        _sw(_lw(mod->text_addr + 0xC69C), mod->text_addr + 0xC99C);

        // Fix index length. This enables CDDA support
        _sw(0x10000014, mod->text_addr + 0x164E4);
    }

    ClearCaches();
}

int patchPopsMan(SceModule2 *mod) {
	SceUID fd = sceIoOpen(sceKernelInitFileName(), PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	// Read header
	PBPHeader header;
	sceIoRead(fd, &header, sizeof(PBPHeader));

	// Get magic
	char magic[16];
	sceIoLseek(fd, header.psar_offset, PSP_SEEK_SET);
	sceIoRead(fd, magic, sizeof(magic));

	if (memcmp(magic, "PSISOIMG0000", 12) == 0) { // Single-Disc
		sceIoLseek(fd, header.psar_offset + 0x400, PSP_SEEK_SET);
	} else if (memcmp(magic, "PSTITLEIMG000000", 16) == 0) { // Multi-Disc
		sceIoLseek(fd, header.psar_offset + 0x200, PSP_SEEK_SET);
	} else {
		sceIoClose(fd);
		return 1;
	}

	// Read PGD buffer
	sceIoRead(fd, pgd_buf, sizeof(pgd_buf));

	// Close fd
	sceIoClose(fd);

	// Check PGD magic
	if (((u32 *)pgd_buf)[0] == 0x44475000)
		original = 1;

	int (* SetCompiledSdkVersion)() = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", 0x358CA1BB);
	if (SetCompiledSdkVersion)
		SetCompiledSdkVersion(0x06060110);

	u32 text_addr = mod->text_addr;

	// Use different mode for SceKermitPocs
	_sw(0x2405000E, text_addr + 0x2030);
	_sw(0x2405000E, text_addr + 0x20F0);
	_sw(0x2405000E, text_addr + 0x21A0);

	// Use different pops register location
	_sw(0x3C014BCD, text_addr + 0x11B4);

	// Patch key function. With this, KEYS.BIN or license files are not required anymore. Also this gives support to custom PSone games
	SetVersionKeyContentId = (void *)text_addr + 0x124;
	REDIRECT_FUNCTION(text_addr + 0x14FC, GetVersionKeyContentIdPatched);

	if (!original) {
		// Patch syscall to use it as deflate decompress
		scePopsManExitVSHKernel = (void *)sctrlHENFindFunction("scePops_Manager", "scePopsMan", 0x0090B2C8);
		sctrlHENPatchSyscall((u32)scePopsManExitVSHKernel, scePopsManExitVSHKernelPatched);

		// Fake dnas drm
		REDIRECT_FUNCTION(text_addr + 0x2308, sceIoOpenPatched);
		REDIRECT_FUNCTION(text_addr + 0x2318, sceIoIoctlPatched);
		REDIRECT_FUNCTION(text_addr + 0x2320, sceIoReadPatched);

		// Dummying amctrl decryption functions
		MAKE_DUMMY_FUNCTION(text_addr + 0xA90, 1);
		_sw(0, text_addr + 0x53C);
	}

	ClearCaches();

	return 0;
}