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
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspdebug.h>
#include <pspinit.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <globals.h>
#include "psid.h"

static unsigned char salt[20] __attribute__((aligned(64)));

/** returns 0 if check failed, 1 if check OK */
int checkPSIDHash(void)
{
    unsigned char psidHashFromReboot[16], psid[16];
    int (*_sceOpenPSIDGetOpenPSID)(unsigned char psid[0x10]), err;
    int (*_sceKernelUtilsSha1Digest)(unsigned char *, unsigned, unsigned char *);
    int (*_sctrlKernelGetPSIDHash)(unsigned char psidHash[16]);
    char modName[20], libName[20];
    unsigned int nid;

    {
#if 0
        modName[0] = 's';
        modName[1] = 'c';
        modName[2] = 'e';
        modName[3] = 'O';
        modName[4] = 'p';
        modName[5] = 'e';
        modName[6] = 'n';
        modName[7] = 'P';
        modName[8] = 'S';
        modName[9] = 'I';
        modName[10] = 'D';
        modName[11] = '_';
        modName[12] = 'S';
        modName[13] = 'e';
        modName[14] = 'r';
        modName[15] = 'v';
        modName[16] = 'i';
        modName[17] = 'c';
        modName[18] = 'e';
        modName[19] = '\0';
        libName[0] = 's';
        libName[1] = 'c';
        libName[2] = 'e';
        libName[3] = 'O';
        libName[4] = 'p';
        libName[5] = 'e';
        libName[6] = 'n';
        libName[7] = 'P';
        libName[8] = 'S';
        libName[9] = 'I';
        libName[10] = 'D';
        libName[11] = '_';
        libName[12] = 'd';
        libName[13] = 'r';
        libName[14] = 'i';
        libName[15] = 'v';
        libName[16] = 'e';
        libName[17] = 'r';
        libName[18] = '\0';

        // 0xC69BEBCE: sceOpenPSIDGetOpenPSID
        _sb(0xC6, ((unsigned)&nid) + 3);
        _sb(0x9B, ((unsigned)&nid) + 2);
        _sb(0xEB, ((unsigned)&nid) + 1);
        _sb(0xCE, ((unsigned)&nid) + 0);
#endif        
        modName[7] = 'P';
        libName[2] = 'e';
        modName[12] = 'S';
        modName[17] = 'c';
        modName[13] = 'e';
        modName[1] = 'c';
        libName[13] = 'r';
        libName[1] = 'c';
        modName[15] = 'v';
        modName[9] = 'I';
        modName[3] = 'O';
        modName[16] = 'i';
        libName[11] = '_';
        libName[10] = 'D';
        libName[17] = 'r';
        libName[9] = 'I';
        libName[7] = 'P';
        modName[18] = 'e';
        libName[18] = '\0';
        libName[15] = 'v';
        modName[14] = 'r';
        libName[12] = 'd';
        _sb(0xC6, ((unsigned)&nid) + 3);
        libName[14] = 'i';
        libName[0] = 's';
        modName[0] = 's';
        libName[3] = 'O';
        modName[2] = 'e';
        libName[4] = 'p';
        modName[5] = 'e';
        modName[10] = 'D';
        _sb(0x9B, ((unsigned)&nid) + 2);
        _sb(0xEB, ((unsigned)&nid) + 1);
        modName[8] = 'S';
        libName[16] = 'e';
        libName[6] = 'n';
        libName[5] = 'e';
        libName[8] = 'S';
        _sb(0xCE, ((unsigned)&nid) + 0);
        modName[11] = '_';
        modName[6] = 'n';
        modName[4] = 'p';
        modName[19] = '\0';

        _sceOpenPSIDGetOpenPSID = (void*)sctrlHENFindFunction(modName, libName, nid);

        if(_sceOpenPSIDGetOpenPSID == NULL)
        {
            return 0;
        }

#if 0
        modName[0] = 'S';
        modName[1] = 'y';
        modName[2] = 's';
        modName[3] = 't';
        modName[4] = 'e';
        modName[5] = 'm';
        modName[6] = 'C';
        modName[7] = 'o';
        modName[8] = 'n';
        modName[9] = 't';
        modName[10] = 'r';
        modName[11] = 'o';
        modName[12] = 'l';
        modName[13] = '\0';
        libName[0] = 'S';
        libName[1] = 'y';
        libName[2] = 's';
        libName[3] = 't';
        libName[4] = 'e';
        libName[5] = 'm';
        libName[6] = 'C';
        libName[7] = 't';
        libName[8] = 'r';
        libName[9] = 'l';
        libName[10] = 'F';
        libName[11] = 'o';
        libName[12] = 'r';
        libName[13] = 'K';
        libName[14] = 'e';
        libName[15] = 'r';
        libName[16] = 'n';
        libName[17] = 'e';
        libName[18] = 'l';
        libName[19] = '\0';

        // 0xCE2F0F74: sctrlKernelGetPSIDHash
        _sb(0xCE, ((unsigned)&nid) + 3);
        _sb(0x2F, ((unsigned)&nid) + 2);
        _sb(0x0F, ((unsigned)&nid) + 1);
        _sb(0x74, ((unsigned)&nid) + 0);
#endif

        _sb(0xCE, ((unsigned)&nid) + 3);
        libName[19] = '\0';
        _sb(0x2F, ((unsigned)&nid) + 2);
        modName[1] = 'y';
        libName[5] = 'm';
        modName[8] = 'n';
        libName[15] = 'r';
        libName[11] = 'o';
        libName[16] = 'n';
        libName[8] = 'r';
        libName[14] = 'e';
        modName[3] = 't';
        modName[11] = 'o';
        modName[7] = 'o';
        libName[18] = 'l';
        modName[13] = '\0';
        modName[5] = 'm';
        modName[9] = 't';
        modName[6] = 'C';
        libName[3] = 't';
        libName[9] = 'l';
        libName[2] = 's';
        modName[12] = 'l';
        modName[0] = 'S';
        modName[10] = 'r';
        _sb(0x0F, ((unsigned)&nid) + 1);
        libName[6] = 'C';
        _sb(0x74, ((unsigned)&nid) + 0);
        libName[17] = 'e';
        modName[2] = 's';
        libName[1] = 'y';
        libName[13] = 'K';
        libName[12] = 'r';
        libName[7] = 't';
        libName[0] = 'S';
        libName[4] = 'e';
        modName[4] = 'e';
        libName[10] = 'F';

        _sctrlKernelGetPSIDHash = (void*)sctrlHENFindFunction(modName, libName, nid);

        if(_sctrlKernelGetPSIDHash == NULL)
        {
            return 0;
        }
    }

    // Get PSID from systemctrl
    err = (*_sctrlKernelGetPSIDHash)(psidHashFromReboot);

    if(err < 0)
    {
        return 0;
    }

    err = (*_sceOpenPSIDGetOpenPSID)(psid);
    
    if (err < 0)
    {
        return 0;
    }

    // hashing...
    _sw(PSID_SALT_MAGIC, (unsigned int)salt);
    memcpy(salt+4, psid, 16);

    {
#if 0
        libName[0] = 'U';
        libName[1] = 't';
        libName[2] = 'i';
        libName[3] = 'l';
        libName[4] = 's';
        libName[5] = 'F';
        libName[6] = 'o';
        libName[7] = 'r';
        libName[8] = 'K';
        libName[9] = 'e';
        libName[10] = 'r';
        libName[11] = 'n';
        libName[12] = 'e';
        libName[13] = 'l';
        libName[14] = '\0';
        
        // 0x840259F1: sceKernelUtilsSha1Digest UtilsForUser sceSystemMemoryManager
        _sb(0x84, ((unsigned)&nid) + 3);
        _sb(0x02, ((unsigned)&nid) + 2);
        _sb(0x59, ((unsigned)&nid) + 1);
        _sb(0xF1, ((unsigned)&nid) + 0);
#endif
        _sb(0x02, ((unsigned)&nid) + 2);
        libName[4] = 's';
        libName[13] = 'l';
        libName[12] = 'e';
        _sb(0x84, ((unsigned)&nid) + 3);
        libName[11] = 'n';
        libName[9] = 'e';
        _sb(0x59, ((unsigned)&nid) + 1);
        libName[10] = 'r';
        libName[8] = 'K';
        _sb(0xF1, ((unsigned)&nid) + 0);
        libName[7] = 'r';
        libName[1] = 't';
        libName[2] = 'i';
        libName[14] = '\0';
        libName[5] = 'F';
        libName[0] = 'U';
        libName[3] = 'l';
        libName[6] = 'o';

        _sceKernelUtilsSha1Digest = (void*)sctrlHENFindFunction((void*)0x88000000, libName, nid);

        if(_sceKernelUtilsSha1Digest == NULL)
        {
            return 0;
        }
    }

    err = _sceKernelUtilsSha1Digest(salt, sizeof(salt), salt);

    if (err < 0)
        return 0;

    if(0 != memcmp(salt, psidHashFromReboot, sizeof(psidHashFromReboot)))
    {
        return 0;
    }

    return 1;
}

static int killThread(SceSize args, void *argp)
{
    while (1)
    {
        unsigned int ret, rand;
        int i;

        rand = sctrlKernelRand();
        ret = (0x88000000 + (rand % 0x400000)) & 0xFFFFFFF0;

        for(i=0; i<16; i+=4)
        {
            _sw(rand, ret+i);
        }

        sceKernelDelayThread(1 * 1000000L);
    }

    return 0;
}

void startKillThread(void)
{
    SceUID thid;

    thid = sceKernelCreateThread("", &killThread, 0x10, 0x1000, 0, 0);

    if (thid >= 0) {
        sceKernelStartThread(thid, 0, NULL);
    }
}

void psidCheck(void)
{
    if(!checkPSIDHash())
    {
        startKillThread();
    }
}
