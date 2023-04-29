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
#include <pspreg.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspumd.h>
#include <psprtc.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "inferno.h"
#include "globals.h"
#include "macros.h"

extern int sceKernelGetCompiledSdkVersion(void);
extern int sceKernelCancelEventFlag(SceUID evf, SceUInt new_value, int *num_wait_threads);

static void do_umd_notify(int arg);

// 0x000027AC
int g_umd_error_status = 0;

// 0x000027B0
int g_drive_status = 0;

// 0x00002794
u32 g_prev_gp = 0;
int (*g_00002798)(int, int, int) = 0;
u32 g_0000279C = 0;
u32 g_000027A0 = 0;

// 0x000027A4
SceUID g_umd_cbid = 0;

SceUID g_drive_status_evf = -1;

int g_disc_type = PSP_UMD_TYPE_GAME;

extern int sceKernelCancelSema(SceUID semaid, int newcount, int *num_wait_threads);

static int check_memory(const void *addr, int size)
{
    const void *end_addr;
    u32 k1;

    k1 = pspSdkGetK1();
    end_addr = addr + size - 1;

    if((int)(((u32)end_addr | (u32)addr) & (k1 << 11)) < 0) {
        return 0;
    }

    return 1;
}

int sceUmdCheckMedium(void)
{
    int ret;
    
    if (g_iso_fn == NULL || g_iso_fn[0] == '\0'){
        return 0;
    }

    while(!g_iso_opened) {
        sceKernelDelayThread(10000);
    }

    ret = 1;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

int sceUmdReplacePermit(void)
{
    int ret;
    ret = 0;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

int sceUmdReplaceProhibit(void)
{
    int ret;
    ret = 0;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

// 0x00001A14
static void do_umd_notify(int arg)
{
    if(g_umd_cbid < 0) {
        return;
    }

    sceKernelNotifyCallback(g_umd_cbid, arg);
}

int sceUmdRegisterUMDCallBack(int cbid)
{
    int ret, intr;
    u32 k1;

    k1 = pspSdkSetK1(0);
    ret = sceKernelGetThreadmanIdType(cbid);

    if(ret != SCE_KERNEL_TMID_Callback) {
        ret = 0x80010016;
        goto exit;
    }

    intr = sceKernelCpuSuspendIntr();
    g_umd_cbid = cbid;
    sceKernelCpuResumeIntr(intr);
    ret = 0;

exit:
    pspSdkSetK1(k1);

    return ret;
}

int sceUmdUnRegisterUMDCallBack(int cbid)
{
    u32 k1;
    int ret, intr;
    uidControlBlock *type;

    k1 = pspSdkSetK1(0);
    ret = 0x80010016;

    if(sceKernelGetUIDcontrolBlock(cbid, &type) == 0) {
        if(g_umd_cbid == cbid) {
            intr = sceKernelCpuSuspendIntr();
            g_umd_cbid = -1;
            sceKernelCpuResumeIntr(intr);
            ret = 0;
        }
    }

    pspSdkSetK1(k1);

    return ret;
}

int infernoSetDiscType(int type)
{
    int oldtype;

    oldtype = g_disc_type;
    g_disc_type = type;

    return oldtype;
}

int sceUmdGetDiscInfo(pspUmdInfo *info)
{
    int ret;
    u32 k1;

    if(!check_memory(info, sizeof(*info))) {
        ret = 0x80010016;
        goto exit;
    }

    k1 = pspSdkSetK1(0);

    if(info != NULL && sizeof(*info) == info->size) {
        info->type = g_disc_type;
        ret = 0;
    } else {
        ret = 0x80010016;
    }

    pspSdkSetK1(k1);

exit:
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

int sceUmdCancelWaitDriveStat(void)
{
    int ret;
    u32 k1;

    k1 = pspSdkSetK1(0);
    ret = sceKernelCancelEventFlag(g_drive_status_evf, g_drive_status, NULL);
    pspSdkSetK1(k1);

    return ret;
}

u32 sceUmdGetErrorStatus(void)
{
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, g_umd_error_status);
    #endif
    return g_umd_error_status;
}

void sceUmdSetErrorStatus(u32 status)
{
    g_umd_error_status = status;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, g_umd_error_status);
    #endif
}

int sceUmdGetDriveStat(void)
{
    return g_drive_status;
}

u32 sceUmdGetDriveStatus(u32 status)
{
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, g_drive_status);
    #endif    
    return g_drive_status;
}

int sceUmdMan_driver_D37B6422(void)
{
    int ret;
    ret = 0;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

int sceUmdMan_driver_6A1FB0DD(void)
{
    int ret;
    ret = 0;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

int sceUmdMan_driver_7DF4C4DA(u32 a0)
{
    if(g_0000279C != a0) {
        return 0x80010002;
    }

    g_prev_gp = 0;
    g_00002798 = 0;
    g_0000279C = 0;
    g_000027A0 = 0;

    return 0;
}

int sceUmdMan_driver_F7A0D0D9(void)
{
    int ret;
    ret = 0;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

static inline u32 get_gp(void)
{
    u32 gp;

    __asm__ volatile ("move %0, $gp;" : "=r"(gp));

    return gp;
}

static inline void set_gp(u32 gp)
{
    __asm__ volatile ("move $gp, %0;" : :"r"(gp));
}

int sceUmdMan_driver_4FFAB8DA(u32 a0, u32 a1, u32 a2)
{
    if(0 != g_0000279C)
    {
        return 0x8001000C;
    }

    g_0000279C = a0;
    g_prev_gp = get_gp();
    g_000027A0 = a2;
    g_00002798 = (void*)a1;
    
    SceModule2* mod = (SceModule2*)sceKernelFindModuleByName("sceIsofs_driver");
    u32 text_addr = mod->text_addr;
    u32 intr = LI_V0(0);

    int patches = 4;
    u32 top_addr = text_addr+mod->text_size;
    for (u32 addr=text_addr; addr<top_addr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x86430048){
            _sw(intr, addr-16);
            patches--;
        }
        else if (data == 0x3C147FDE){
            _sw(intr, addr+8);
            patches--;
        }
        else if (data == 0x8D240018){
            _sw(intr, addr+4);
            patches--;
        }
        else if (data == 0x34C30016){
            _sw(intr, addr-16);
            patches--;
        }
    }
    flushCache();

    if(0 == g_00002798)
    {
        return 0;
    }

    set_gp(g_prev_gp);
    (*g_00002798)(g_0000279C, g_000027A0, 1);

    return 0;
}

// 0x000014F0
void sceUmdClearDriveStatus(u32 mask)
{
    int intr;

    intr = sceKernelCpuSuspendIntr();
    sceKernelClearEventFlag(g_drive_status_evf, mask);
    g_drive_status &= mask;
    sceKernelCpuResumeIntr(intr);
    do_umd_notify(g_drive_status);
}

int sceUmd9660_driver_63342C0F(void)
{
    int ret;
    ret = 0;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

int sceUmd9660_driver_6FFFEE54(void)
{
    int ret;
    ret = 0;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

int sceUmd9660_driver_7CB291E3(void)
{
    int ret;
    ret = 0;
    #ifdef DEBUG
    printk("%s: -> 0x%08X\n", __func__, ret);
    #endif
    return ret;
}

int sceUmdWaitDriveStatWithTimer(int stat, SceUInt timeout)
{
    int ret;
    u32 k1, result;

    if(!(stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY))) {
        return 0x80010016;
    }

    k1 = pspSdkSetK1(0);
    ret = sceKernelWaitEventFlag(g_drive_status_evf, stat, PSP_EVENT_WAITOR, &result, timeout == 0 ? NULL : &timeout);
    pspSdkSetK1(0);

    return ret;
}

int sceUmdWaitDriveStatCB(int stat, SceUInt timeout)
{
    int ret;
    u32 k1, result;

    if(!(stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY))) {
        return 0x80010016;
    }

    k1 = pspSdkSetK1(0);
    ret = sceKernelWaitEventFlagCB(g_drive_status_evf, stat, PSP_EVENT_WAITOR, &result, timeout == 0 ? NULL : &timeout);
    pspSdkSetK1(0);

    return ret;
}

int sceUmdWaitDriveStat(int stat)
{
    int ret;
    u32 k1, result;

    if(!(stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY))) {
        return 0x80010016;
    }

    k1 = pspSdkSetK1(0);
    ret = sceKernelWaitEventFlag(g_drive_status_evf, stat, PSP_EVENT_WAITOR, &result, NULL);
    pspSdkSetK1(0);

    return ret;
}

int sceUmdActivate(int unit, const char* drive)
{
    u32 k1;
    int value;

    if(!g_iso_opened) {
        return 0x80010016;
    }

    if(drive == NULL || !check_memory(drive, strlen(drive) + 1)) {
        return 0x80010016;
    }

    k1 = pspSdkSetK1(0);

    if(0 != strcmp(drive, "disc0:")) {
        pspSdkSetK1(k1);

        return 0x80010016;
    }

    value = 1;
    sceIoAssign(drive, "umd0:", "isofs0:", 1, &value, sizeof(value));
    sceUmdSetDriveStatus(PSP_UMD_PRESENT | PSP_UMD_INITED | PSP_UMD_READY);

    if(g_game_fix_type == 1) {
        do_umd_notify(PSP_UMD_PRESENT | PSP_UMD_READY);
        pspSdkSetK1(k1);

        return 0;
    }

    if(g_drive_status & PSP_UMD_READY) {
        pspSdkSetK1(k1);

        return 0;
    }

    do_umd_notify(g_drive_status);
    pspSdkSetK1(k1);

    return 0;
}

int sceUmdDeactivate(int unit, const char *drive)
{
    int ret;
    u32 k1;

    if(drive == NULL || !check_memory(drive, strlen(drive) + 1)) {
        return 0x80010016;
    }

    k1 = pspSdkSetK1(0);
    ret = sceIoUnassign(drive);

    if(ret < 0) {
        pspSdkSetK1(k1);

        return ret;
    }

    sceUmdSetDriveStatus(PSP_UMD_PRESENT | PSP_UMD_INITED);
    pspSdkSetK1(k1);

    return ret;
}

int sceUmdGetErrorStat(void)
{
    u32 k1;
    int ret;

    k1 = pspSdkSetK1(0);
    ret = g_umd_error_status;
    pspSdkSetK1(k1);

    return ret;
}

// 0x000018A4
// call @PRO_Inferno_Driver:sceUmd,0xF60013F8@
void sceUmdSetDriveStatus(int status)
{
    int intr;

    intr = sceKernelCpuSuspendIntr();

    if(!(status & PSP_UMD_NOT_PRESENT)) {
        if(status & (PSP_UMD_PRESENT | PSP_UMD_CHANGED | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY)) {
            g_drive_status &= ~PSP_UMD_NOT_PRESENT;
        }
    } else {
        g_drive_status &= ~(PSP_UMD_PRESENT | PSP_UMD_CHANGED | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY);
    }

    if(!(status & PSP_UMD_INITING)) {
        if(status & (PSP_UMD_INITED | PSP_UMD_READY)) {
            g_drive_status &= ~PSP_UMD_INITING;
        }
    } else {
        g_drive_status &= ~(PSP_UMD_INITED | PSP_UMD_READY);
    }

    if(!(status & PSP_UMD_READY)) {
        g_drive_status &= ~PSP_UMD_READY;
    }

    g_drive_status |= status;

    if(g_drive_status & PSP_UMD_READY) {
        g_drive_status |= PSP_UMD_INITED;
    }

    if(g_drive_status & PSP_UMD_INITED) {
        g_drive_status |= PSP_UMD_PRESENT;
        sceUmdSetErrorStatus(0);
    }

    sceKernelSetEventFlag(g_drive_status_evf, g_drive_status);
    sceKernelCpuResumeIntr(intr);
    do_umd_notify(g_drive_status);
}

int sceUmd_004F4BE5(int orig_error_code)
{
    u32 compiled_version;
    int error_code = orig_error_code;

    if(error_code == 0) {
        goto exit;
    }

    compiled_version = sceKernelGetCompiledSdkVersion();

    if(compiled_version == 0) {
        if(error_code == 0x80010074) {
            error_code = 0x8001006E;
        } else if(error_code == 0x80010070) {
            error_code = 0x80010062;
        } else if(error_code == 0x8001005B) {
            error_code = 0x80010062;
        } else if(error_code == 0x80010071) {
            error_code = 0x80010067;
        } else if(error_code == 0x80010086) {
            error_code = 0x8001B000;
        } else if(error_code == 0x80010087) {
            error_code = 0x8001007B;
        } else if(error_code == 0x8001B006) {
            error_code = 0x8001007C;
        }
    }

exit:
    return error_code;
}

static u32 g_unk_data = 0;

/* Used by vshbridge */
u32 sceUmd_107064CC(void)
{
    return g_unk_data;
}

void sceUmd_C886430B(u32 a0)
{
    g_unk_data = a0;
}

int power_event_handler(int ev_id, char *ev_name, void *param, int *result)
{
    //static int old_status;

    if( ev_id == 0x400 )
	{
		//do_umd_notify( 0x9 );
        //old_status = g_drive_status;
        do_umd_notify(PSP_UMD_INITING);
	}

    if(ev_id == 0x400000) { // resume complete
        do_umd_notify(0x32);
        //do_umd_notify(old_status);
    }

    return 0;
}
