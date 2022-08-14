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
#include "macros.h"
#include "module2.h"

#define REGION_DEBUG_CODE 1

#define REGION_EUROPE 0x60
#define REGION_JAPAN 0x78


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

static int (*IdStorageLookup)(u16 key, u32 offset, void *buf, u32 len);
static int fakeIdStorageLookupForRegion(u16 key, u32 offset, void *buf, u32 len){
    if (key == 258 && offset == 140 && len == 4){
        memset(buf, 0, len);
        *(u8*)buf = REGION_JAPAN;
        return 0;
    }
    return IdStorageLookup(key, offset, buf, len);
}

void patch_region(void)
{
	// sceChkregGetPsCode
	u32 fp = sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x59F8491D); 
	if (fp) {
        _sw(JUMP(_sceChkregGetPsCode), fp);
        _sw(NOP, fp+4);
    }

    IdStorageLookup = sctrlHENFindFunction("sceIdStorage_Service", "sceIdStorage_driver", 0x6FE062D1);
    SceModule2* mod = sceKernelFindModuleByName("sceUmdMan_driver");
    if (mod){
        hookImportByNID(mod, "sceIdStorage_driver", 0x6FE062D1, &fakeIdStorageLookupForRegion);
    }
}