#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysevent.h>
#include <psploadcore.h>
#include <pspiofilemgr_kernel.h>
#include <pspsyscon.h>
#include <pspnand_driver.h>
#include <pspidstorage.h>
#include <pspdisplay_kernel.h>
#include <systemctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


PSP_MODULE_INFO("IoPrivileged", 0x1007, 1, 0);


SceUID sceIoOpenPrivileged(const char *file, int flags, SceMode mode)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoOpen(file, flags, mode);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoClosePrivileged(SceUID fd)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoClose(fd);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoReadPrivileged(SceUID fd, void *data, SceSize size)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoRead(fd, data, size);
    
    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoWritePrivileged(SceUID fd, const void *data, SceSize size)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoWrite(fd, data, size);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

SceOff sceIoLseekPrivileged(SceUID fd, SceOff offset, int whence)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    SceOff res = sceIoLseek(fd, offset, whence);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoLseek32Privileged(SceUID fd, int offset, int whence)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    SceOff res = sceIoLseek32(fd, offset, whence);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoRenamePrivileged(const char *oldname, const char *newname)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoRename(oldname, newname);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoRemovePrivileged(const char *file)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoRemove(file);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoGetstatPrivileged(const char *file, SceIoStat *stat)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoGetstat(file, stat);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoChstatPrivileged(const char *file, SceIoStat *stat, int bits)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoChstat(file, stat, bits);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

SceUID sceIoDopenPrivileged(const char *dirname)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoDopen(dirname);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoDreadPrivileged(SceUID fd, SceIoDirent *dir)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoDread(fd, dir);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoDclosePrivileged(SceUID fd)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoDclose(fd);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoChdirPrivileged(const char *path)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoChdir(path);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoMkdirPrivileged(const char *dir, SceMode mode)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoMkdir(dir, mode);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoRmdirPrivileged(const char *path)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoRmdir(path);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoIoctlPrivileged(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoIoctl(fd, cmd, indata, inlen, outdata, outlen);

    pspSdkSetK1(k1);
    sctrlKernelSetUserLevel(level);
    return res;
}

int sceIoDevctlPrivileged(const char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoDevctl(dev, cmd, indata, inlen, outdata, outlen);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoSyncPrivileged(const char *device, unsigned int unk)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoSync(device, unk);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoAssignPrivileged(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoAssign(dev1, dev2, dev3, mode, unk1, unk2);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int sceIoUnassignPrivileged(const char *dev)
{
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);

    int res = sceIoUnassign(dev);

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return res;
}

int module_start(SceSize args, void *argp)
{
    void (* PatchSyscall)(u32 funcaddr, void *newfunc);
    PatchSyscall = (void *)sctrlHENFindFunction("SystemControl", "SystemCtrlForKernel", 0x826668E9);
    
    if (!PatchSyscall)
    {
        PatchSyscall = (void *)sctrlHENFindFunction("SystemControl", "SystemCtrlForKernel", 0x02BFCB5F);

        if (!PatchSyscall)
        {
        	asm("break\n");
        	return 1;
        }
    }
    
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x06A70004), sceIoMkdirPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x109F50BC), sceIoOpenPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x1117C65F), sceIoRmdirPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x27EB27B8), sceIoLseekPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x42EC03AC), sceIoWritePrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x54F5FB11), sceIoDevctlPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x55F4717D), sceIoChdirPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x63632449), sceIoIoctlPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x6A638D83), sceIoReadPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x6D08A871), sceIoUnassignPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x779103A0), sceIoRenamePrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x810C4BC3), sceIoClosePrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xAB96437F), sceIoSyncPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xACE946E8), sceIoGetstatPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xB29DDF9C), sceIoDopenPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xB2A628C1), sceIoAssignPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xB8A740F4), sceIoChstatPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xE3EB004C), sceIoDreadPrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xEB092469), sceIoDclosePrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xF27A9C51), sceIoRemovePrivileged);
    PatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x68963324), sceIoLseek32Privileged);
    
    
    sceKernelDcacheWritebackAll();
    sceKernelIcacheClearAll();

    return 0;
}

