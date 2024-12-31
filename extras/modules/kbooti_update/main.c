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
#include <pspdebug.h>
#include <psploadcore.h>
#include <pspiofilemgr.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PSP_MODULE_INFO("kbooti_update", 0x1006, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

u32 kbooti_350_xor_key[] =
{
    0x13DE820E, 0x3EB24BA8, 0x5471EC1F, 0x5AC45321,
    0x6A9B7DA9, 0x1D761B46, 0xE521B9D1, 0x4F8DE094,
    0x052C4096, 0x700D6624, 0xB0FE8F0C, 0x0E3ED589,
    0x0ECE9063, 0xA5CB715E, 0x14539181, 0x74343E99
};

#define KBOOTI_BIN_PATH "flash3:/kbooti_01g.bin"
#define KBOOTI_BIN_BACKUP_PATH "flash3:/kbooti_01g.bin.bak"

#define KBOOTI_SIZE 0x21000
#define CKBOOTI_SIZE 0x23D10

#define LIB_PSP_IPL_LOADER_SIZE 0x1000
#define IPL_BLOCK_SIZE 0x1000
#define KIRK1_HEADER_SIZE 0x40

#define error_exit() \
    if (ret < 0) {   \
        goto error;  \
    }

static u8 g_buf[IPL_BLOCK_SIZE + 0x40] __attribute__((aligned(64)));
static u8 g_buf2[IPL_BLOCK_SIZE + 0x40] __attribute__((aligned(64)));
static u8 kbooti_buf[0x24000] __attribute__((aligned(64)));

int (*sceUtilsBufferCopyWithRange)(void* outBuf, int outSize, void* inBuf, int inSize, int command);

void descramble_kirk_header(u32 *kirk_header)
{
    for (int i = 0; i < KIRK1_HEADER_SIZE/4; i++) {
        kirk_header[i] = kirk_header[i] ^ kbooti_350_xor_key[i];
    }
}

void copy_kbooti_to_tachsm()
{
    int fd = -1, size;

    fd = sceIoOpen(KBOOTI_BIN_PATH, PSP_O_RDONLY , 0777);

    if (fd < 0) {
        goto exit;
    }

    size = sceIoRead(fd, kbooti_buf, sizeof(kbooti_buf));

    if (size < 0) {
        goto exit;
    }

    memcpy((void*)0xBFE00000, kbooti_buf, size);

exit:
    if (fd > 0) {
        sceIoClose(fd);
    }

}

int rename(const char *oldname, const char* newname)
{
    int ret;
    SceUID fd, fd2;
    SceIoStat srcstat;

    ret = sceIoGetstat(newname, &srcstat);

    if (ret != 0x80010002) { //SCE_ERROR_ERRNO_FILE_NOT_FOUND
        return -1;
    }

    ret = sceIoOpen(oldname, PSP_O_RDONLY , 0777);
    error_exit();
    fd = ret;

    ret = sceIoOpen(newname, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC , 0777);
    error_exit();
    fd2 = ret;

    ret = sceIoRead(fd, g_buf, 0x1000);

    while (ret > 0) {
        sceIoWrite(fd2, g_buf, ret);
        error_exit();
        ret = sceIoRead(fd, g_buf, 0x1000);
    }

    error_exit();

    sceIoClose(fd);
    sceIoClose(fd2);

    sceIoRemove(oldname);
    error_exit();

    return 1;

error:
    if (fd > 0) {
        sceIoClose(fd);
    }
    if (fd2 > 0) {
        sceIoClose(fd2);
    }

    return ret;
}

void init_flash()
{
    int ret;
   
    ret = sceIoUnassign("flash3:");

    while(ret < 0 && ret != SCE_KERNEL_ERROR_NODEV) {
        ret = sceIoUnassign("flash3:");
        sceKernelDelayThread(500000);
    }

    sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", 0, NULL, 0);
}

int pspKbootiUpdateGetKbootiSize()
{
    int k1 = pspSdkSetK1(0);

    SceIoStat srcstat;

    init_flash();

    int ret = sceIoGetstat(KBOOTI_BIN_PATH, &srcstat);

    if (ret < 0)
        goto exit;

    ret = srcstat.st_size;

exit:
    pspSdkSetK1(k1);

    return ret; 
}

int pspKbootiUpdateKbooti(u8 *cIplBlock, u32 cIplBlockSize)
{    
    int k1 = pspSdkSetK1(0);
    
    SceUID fd = -1;
    int ret;
    int kbootiBinBackedUp = 0;
    int count = 0;
    SceIoStat srcstat;

    memset(kbooti_buf, 0, sizeof(kbooti_buf));

    ret = sceIoGetstat(KBOOTI_BIN_BACKUP_PATH, &srcstat);

    if (ret == 0x80010002) { //SCE_ERROR_ERRNO_FILE_NOT_FOUND
        ret = sceIoOpen(KBOOTI_BIN_PATH, PSP_O_RDONLY , 0777);
    }
    else if (ret >= 0) {
        kbootiBinBackedUp = 1;
        ret = sceIoOpen(KBOOTI_BIN_BACKUP_PATH, PSP_O_RDONLY , 0777);
    }
    else {
        goto error;
    }

    error_exit();
    fd = ret;

    //read Lib PSP iplloader from kbooti.bin
    ret = sceIoRead(fd, kbooti_buf, LIB_PSP_IPL_LOADER_SIZE);

    if (ret != LIB_PSP_IPL_LOADER_SIZE) {
        ret = ret > 0 ? -1 : ret;
        goto error;
    }

    count += LIB_PSP_IPL_LOADER_SIZE;

    //Copy custom ipl block
    memcpy(kbooti_buf + count, cIplBlock, cIplBlockSize);
    count += cIplBlockSize;

    ret = sceIoRead(fd, g_buf + 0x40, IPL_BLOCK_SIZE);

    while (ret > 0) {
        descramble_kirk_header(g_buf + 0x40);

        //decrypt ipl block
        ret = sceUtilsBufferCopyWithRange(g_buf2, sizeof(g_buf2), g_buf + 0x40, IPL_BLOCK_SIZE, 1);

        error_exit();

        u32 size = ((u32 *)g_buf2)[1];
        memcpy(kbooti_buf + count, g_buf2 + 0x10, size);
        count += size;

        ret = sceIoRead(fd, g_buf + 0x40, IPL_BLOCK_SIZE);
    }

    sceIoClose(fd);

    if (count != CKBOOTI_SIZE) {
        ret = -1;
        goto error;
    }

    if (!kbootiBinBackedUp) {
        ret = rename(KBOOTI_BIN_PATH, KBOOTI_BIN_BACKUP_PATH);
        error_exit();
    }

    ret = sceIoOpen(KBOOTI_BIN_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC , 0777);
    error_exit();
    fd = ret;

    ret = sceIoWrite(fd, kbooti_buf, count);

    if (ret != count) {
        ret = ret > 0 ? -1 : ret;
    }

    sceIoClose(fd);

    copy_kbooti_to_tachsm();

    pspSdkSetK1(k1);
    return 1;

error:
    if (fd > 0) {
        sceIoClose(fd);
    }

    pspSdkSetK1(k1);
    return ret;
}

int pspKbootiUpdateRestoreKbooti()
{    
    int k1 = pspSdkSetK1(0);

    int ret;
    SceIoStat srcstat;

    ret = sceIoGetstat(KBOOTI_BIN_BACKUP_PATH, &srcstat);
    error_exit();

    if (srcstat.st_size != KBOOTI_SIZE) {
        ret = -1;
        goto error;
    }

    ret = sceIoRemove(KBOOTI_BIN_PATH);
    error_exit();

    ret = rename(KBOOTI_BIN_BACKUP_PATH, KBOOTI_BIN_PATH);
    error_exit();

    copy_kbooti_to_tachsm();

    pspSdkSetK1(k1);
    return 1;

error:
    pspSdkSetK1(k1);
    return ret;
}

int module_start(SceSize args, void *argp)
{
    sceUtilsBufferCopyWithRange = (void*)sctrlHENFindFunction("sceMemlmd", "semaphore", 0x4C537C72);

    if (sceUtilsBufferCopyWithRange == NULL)
        return -1;

    return 0;
}

int module_stop(void)
{
    return 0;
}
