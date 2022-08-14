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

#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "macros.h"
#include "module2.h"
#include "region_free.h"

extern ARKConfig* ark_config;

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
	TA_096_TA_097,
	UNKNOWN
};

int region_change = 0;

static int (*IdStorageLookup)(u16 key, u32 offset, void *buf, u32 len);
static void* umd_buf = NULL;
static u16 umd_key = 0;

static int _sceChkregGetPsCode(u8 *pscode)
{
	pscode[0] = 1;
	pscode[1] = 0;
	pscode[2] = REGION_DEBUG_CODE;
	pscode[3] = 0;
	pscode[4] = 1;
	pscode[5] = 0;
	pscode[6] = 1;
	pscode[7] = 0;

	return 0;
}

static int fakeIdStorageLookupForUmd(u16 key, u32 offset, void *buf, u32 len){
    if (offset == 0 && len==512 && umd_buf == NULL){ // obtain buffer where UMD keys are stored in umdman.prx
        umd_buf = buf;
        umd_key = key;
    }
    return IdStorageLookup(key, offset, buf, len); // passthrough
}

int GetHardwareInfo(u32 *ptachyon, u32 *pbaryon, u32 *ppommel, u32 *pmb, u64 *pfuseid)
{
	u32 tachyon = 0;
	u32 baryon = 0;
	u32 pommel = 0;
	u32 mb = UNKNOWN;
    u64 fuseid;

    u32 (*SysregGetTachyonVersion)() = sctrlHENFindFunction("sceLowIO_Driver", "sceSysreg_driver", 0xE2A5D1EE);
    u64 (*SysregGetFuseId)() = sctrlHENFindFunction("sceLowIO_Driver", "sceSysreg_driver", 0x4F46EEDE);
    void (*SysconGetBaryonVersion)(u32*) = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7EC5A957);
    void (*SysconGetPommelVersion)(u32*) = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0xE7E87741);

    if (SysregGetTachyonVersion == NULL || SysregGetFuseId == NULL || SysconGetBaryonVersion == NULL || SysconGetPommelVersion == NULL) return -1;

	tachyon = SysregGetTachyonVersion();
	SysconGetBaryonVersion(&baryon);
	SysconGetPommelVersion(&pommel);
    fuseid = SysregGetFuseId();
	
	switch (tachyon)
	{
		case 0x00140000:
			switch(baryon)
			{
				case 0x00010600: mb = TA_079v1; break;
				case 0x00010601: mb = TMU_001v1; break;
				case 0x00020600: mb = TA_079v2; break;
				case 0x00020601: mb = TMU_001v2; break;
				case 0x00030600: mb = TA_079v3; break;
				case 0x00030601: mb = TMU_002; break;
			}
			break;

		case 0x00200000:
			switch(baryon)
			{
				case 0x00030600: mb = TA_079v4; break;
				case 0x00040600: mb = TA_079v5; break;
			}
			break;

		case 0x00300000:
			switch(baryon)
			{
				case 0x00040600:
					switch(pommel)
					{
						case 0x00000103: mb = TA_081v1; break;
						case 0x00000104: mb = TA_081v2; break;
					}
					break;
			}
			break;

		case 0x00400000:
			switch(baryon)
			{
				case 0x00114000: mb = TA_082; break;
				case 0x00121000: mb = TA_086; break;
			}
			break;

		case 0x00500000:
			switch(baryon)
			{
				case 0x0022B200: mb = TA_085v1; break;
				case 0x00234000: mb = TA_085v2; break;
				case 0x00243000:
					switch(pommel)
					{
						case 0x00000123: mb = TA_088v1_TA_088v2; break;
						case 0x00000132: mb = TA_090v1; break;
					}
					break;
			}
			break;

		case 0x00600000:
			switch(baryon)
			{
				case 0x00243000: mb = TA_088v3; break;
				case 0x00263100:
					switch(pommel)
					{
						case 0x00000132: mb = TA_090v2; break;
						case 0x00000133: mb = TA_090v3; break;
					}
					break;
				case 0x00285000: mb = TA_092; break;
			}
			break;

		case 0x00720000: mb = TA_091; break;

		case 0x00800000: mb = TA_094; break;

		case 0x00810000:
			switch(baryon)
			{
				case 0x002C4000:
					switch(pommel)
					{
						case 0x00000141: mb = TA_093v1; break;
						case 0x00000143: mb = TA_093v2; break;
					}
					break;
				case 0x002E4000: mb = TA_095v1; break;
			}
			break;


		case 0x00820000: mb = TA_095v2; break;

		case 0x00900000: mb = TA_096_TA_097; break;
	}

    *ptachyon = tachyon;
    *pbaryon = baryon;
    *ppommel = pommel;
    *pmb = mb;
    *pfuseid = fuseid;
    return 0;
}

int replace_umd_keys(){
    int res = -1;

    // allocate memory buffer
    SceUID memid = sceKernelAllocPartitionMemory(2, "idsBuffer", PSP_SMEM_High, 256*1024, NULL);
    void* big_buffer = sceKernelGetBlockHeadAddr(memid);

    if (memid < 0 || big_buffer == NULL) goto fake_ids_end;

    // load and start idsRegeneration module
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "IDSREG.PRX");
    SceUID modid = sceKernelLoadModule(path, 0, NULL);

    if (modid < 0) goto fake_ids_end;

    int r = sceKernelStartModule(modid, strlen(path) + 1, path, NULL, NULL);

    if (r < 0) goto fake_ids_end;

    // initialize idsRegeneration and calculate the new UMD keys
    int (*idsRegenerationSetup)(u32, u32, u32, u32, u64, u32, void*) = 
        sctrlHENFindFunction("pspIdsRegeneration_Driver", "idsRegeneration", 0xBDE13E76);
    int (*idsRegenerationCreateCertificatesAndUMDKeys)(void*) = 
        sctrlHENFindFunction("pspIdsRegeneration_Driver", "idsRegeneration", 0xB79A6C46);

    if (idsRegenerationCreateCertificatesAndUMDKeys == NULL || idsRegenerationSetup == NULL) goto fake_ids_end;

    u32 tachyon, baryon, pommel, mb, region;
    u64 fuseid;

    if (GetHardwareInfo(&tachyon, &baryon, &pommel, &mb, &fuseid) < 0) goto fake_ids_end;

    idsRegenerationSetup(tachyon, baryon, pommel, mb, fuseid, region_change, NULL);

    idsRegenerationCreateCertificatesAndUMDKeys(big_buffer);

    // copy the generated UMD keys to the buffer in umdman
    memcpy(umd_buf, big_buffer+(0x200*2), 512*5);
    res = 0;

    // free resources
    fake_ids_end:
    sceKernelFreePartitionMemory(memid);
    sceKernelStopModule(modid, 0, NULL, NULL, NULL);
    sceKernelUnloadModule(modid);

    return res;
}

void patch_umd_idslookup(SceModule2* mod){
    // this patch allows us to obtain the buffer where umdman stores the UMD keys
    IdStorageLookup = sctrlHENFindFunction("sceIdStorage_Service", "sceIdStorage_driver", 0x6FE062D1);
    if (mod){
        hookImportByNID(mod, "sceIdStorage_driver", 0x6FE062D1, &fakeIdStorageLookupForUmd);
    }
}

void patch_region(void)
{
	// sceChkregGetPsCode
	u32 fp = sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x59F8491D); 
	if (fp) {
        _sw(JUMP(_sceChkregGetPsCode), fp);
        _sw(NOP, fp+4);
    }

}

int patch_umd_thread(SceSize args, void *argp){
    sceKernelDelayThread(3000000); // wait for system to load
    replace_umd_keys(); // replace UMD keys
    sceKernelExitDeleteThread(0);
    return 0;
}