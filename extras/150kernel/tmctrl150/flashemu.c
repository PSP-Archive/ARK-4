#include <string.h>

#include <pspiofilemgr_kernel.h>
#include <pspsysevent.h>
#include <pspthreadman_kernel.h>

#include "flashemu.h"
#include "ark.h"

char path[260];

SceUID flashemu_sema;
SceUID flashemuThid;

int msNotReady = 1;
int installed = 0;

extern PspSysEventHandler sysEventHandler;

#define Lock() sceKernelWaitSema(flashemu_sema, 1, NULL)
#define UnLock() sceKernelSignalSema(flashemu_sema, 1)

#define SCE_ERROR_ERRNO150_ENOTSUP 0x8001B000

static int FlashEmu_IoInit();
static int FlashEmu_IoExit();
static int FlashEmu_IoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
static int FlashEmu_IoClose(PspIoDrvFileArg *arg);
static int FlashEmu_IoRead(PspIoDrvFileArg *arg, char *data, int len);
static int FlashEmu_IoWrite(PspIoDrvFileArg *arg, const char *data, int len);
static SceOff FlashEmu_IoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence);
static int FlashEmu_IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
static int FlashEmu_IoRemove(PspIoDrvFileArg *arg, const char *name);
static int FlashEmu_IoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode);
static int FlashEmu_IoRmdir(PspIoDrvFileArg *arg, const char *name);
static int FlashEmu_IoDopen(PspIoDrvFileArg *arg, const char *dirName);
static int FlashEmu_IoDclose(PspIoDrvFileArg *arg);
static int FlashEmu_IoDread(PspIoDrvFileArg *arg, SceIoDirent *dir);
static int FlashEmu_IoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);
static int FlashEmu_IoChstat(PspIoDrvFileArg *arg, const char* file, SceIoStat *stat, int bits);
static int FlashEmu_IoRename(PspIoDrvFileArg *arg, const char * oldname, const char *newname);
static int FlashEmu_IoChdir(PspIoDrvFileArg *arg, const char *dir);
static int FlashEmu_IoMount();
static int FlashEmu_IoUmount();
static int FlashEmu_IoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
static int FlashEmu_IoUnk21();

static int DummyReturnZero();
static int DummyReturnNotSupported();

int SceLfatfsAssign();

static PspIoDrvFuncs lflashFuncs = {
    DummyReturnZero,    	 //IoInit
    DummyReturnZero,    	 //IoExit
    DummyReturnZero,    	 //IoOpen
    DummyReturnZero,    	 //IoClose
    DummyReturnZero,    	 //IoRead
    DummyReturnZero,    	 //IoWrite
    DummyReturnZero,    	 //IoLseek
    DummyReturnZero,    	 //IoIoctl
    DummyReturnNotSupported, //IoRemove
    DummyReturnNotSupported, //IoMkdir
    DummyReturnNotSupported, //IoRmdir
    DummyReturnNotSupported, //IoDopen
    DummyReturnNotSupported, //IoDclose
    DummyReturnNotSupported, //IoDread
    DummyReturnNotSupported, //IoGetstat
    DummyReturnNotSupported, //IoChstat
    DummyReturnNotSupported, //IoRename
    DummyReturnNotSupported, //IoChdir
    DummyReturnNotSupported, //IoMount
    DummyReturnNotSupported, //IoUmount
    DummyReturnZero,    	 //IoDevctl
    DummyReturnNotSupported //IoUnk21
};

static PspIoDrvFuncs flashFatFuncs = {
    FlashEmu_IoInit,
    FlashEmu_IoExit,
    FlashEmu_IoOpen,
    FlashEmu_IoClose,
    FlashEmu_IoRead,
    FlashEmu_IoWrite,
    FlashEmu_IoLseek,
    FlashEmu_IoIoctl,
    FlashEmu_IoRemove,
    FlashEmu_IoMkdir,
    FlashEmu_IoRmdir,
    FlashEmu_IoDopen,
    FlashEmu_IoDclose,
    FlashEmu_IoDread,
    FlashEmu_IoGetstat,
    FlashEmu_IoChstat,
    FlashEmu_IoRename,
    FlashEmu_IoChdir,
    FlashEmu_IoMount,
    FlashEmu_IoUmount,
    FlashEmu_IoDevctl,
    FlashEmu_IoUnk21
};

static PspIoDrv lflashDrv = {"lflash", 0x4, 0x200, 0, &lflashFuncs};
static PspIoDrv flashFatDrv = {"flashfat", 0x10, 1, "FAT over USB Mass", &flashFatFuncs};

int InstallFlashEmu()
{
    if (!installed)
    {
        sceIoAddDrv(&lflashDrv);
        sceIoAddDrv(&flashFatDrv);
        sceKernelRegisterSysEventHandler(&sysEventHandler);
        flashemuThid = sceKernelCreateThread("SceLfatfsAssign", SceLfatfsAssign, 0x64, 0x1000, 0, NULL);
        if (flashemuThid < 0)
        {
        	return flashemuThid;
        }
        else
        {
        	installed = 1;
        	return sceKernelStartThread(flashemuThid, 0, NULL);
        }
    }

    return -1;
}

int UninstallFlashEmu()
{
    return 0;
}

static void BuildPath(const char *file)
{
    strncpy(path, ARK_DC_PATH "/150", 18);
    strncat(path, file, 256);
}

static int FlashEmu_IoExit()
{
    return 0;
}

static int FlashEmu_IoMount()
{
    return 0;
}

static int FlashEmu_IoUmount()
{
    return 0;
}

static int FlashEmu_IoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
    int res = 0;

    switch (cmd)
    {
    case 0x5802:
        res = 0;

        break;

    default:
        //Kprintf("Unknown devctl command 0x%08X\n", res);
        while (1);
    }

    return res;
}

static int FlashEmu_IoUnk21(PspIoDrvFileArg *arg)
{
    return SCE_ERROR_ERRNO150_ENOTSUP;
}

static int DummyReturnZero()
{
    return 0;
}

static int DummyReturnNotSupported()
{
    return SCE_ERROR_ERRNO150_ENOTSUP;
}

static int chdir_main(int *argv)
{
    const char *path = (const char *)argv[1];

    Lock();
    BuildPath(path);

    int res = sceIoChdir(path);

    UnLock();
    return res;
}

static int FlashEmu_IoChdir(PspIoDrvFileArg *arg, const char *dir)
{
    int argv[2];
    argv[0] = (int)arg;
    argv[1] = (int)dir;

    return sceKernelExtendKernelStack(0x4000, (void *)chdir_main, (void *)argv);
}

static int rename_main(int *argv)
{
    const char *oldname = (const char *)argv[1];
    const char *newname = (const char *)argv[2];

    Lock();
    BuildPath(oldname);

    int res = sceIoRename(oldname, newname);

    UnLock();
    return res;
}

static int FlashEmu_IoRename(PspIoDrvFileArg *arg, const char *oldname, const char *newname)
{
    int argv[3];

    argv[0] = (int)arg;
    argv[1] = (int)oldname;
    argv[2] = (int)newname;

    return sceKernelExtendKernelStack(0x4000, (void *)rename_main, (void *)argv);
}

static int chstat_main(int *argv)
{
    const char *file = (const char *)argv[1];
    SceIoStat *stat = (SceIoStat *)argv[2];
    int bits = argv[3];

    Lock();
    BuildPath(file);

    int res = sceIoChstat(path, stat, bits);

    UnLock();
    return res;
}

static int FlashEmu_IoChstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits)
{
    int argv[4];

    argv[0] = (int)arg;
    argv[1] = (int)file;
    argv[2] = (int)stat;
    argv[3] = (int)bits;

    return sceKernelExtendKernelStack(0x4000, (void *)chstat_main, (void *)argv);
}

static int getstat_main(int *argv)
{
    const char *file = (const char *)argv[1];
    SceIoStat *stat = (SceIoStat *)argv[2];

    Lock();
    BuildPath(file);

    int res = sceIoGetstat(path, stat);

    UnLock();

    return res;
}

static int FlashEmu_IoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat)
{
    int argv[3];

    argv[0] = (int)arg;
    argv[1] = (int)file;
    argv[2] = (int)stat;

    return sceKernelExtendKernelStack(0x4000, (void *)getstat_main, (void *)argv);
}

static int dread_main(int *argv)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
    SceIoDirent *dir = (SceIoDirent *)argv[1];

    Lock();
    int res = sceIoDread((SceUID)arg->arg, dir);
    UnLock();

    return res;
}

static int dclose_main(PspIoDrvFileArg *arg)
{
    Lock();

    int res = sceIoDclose((SceUID)arg->arg);
    if (res < 0)
    {
        //Kprintf("Error 0x%08X in IoDclose.\n", res);
    }
    else
    {
        res = 0;
    }

    arg->arg = (void *)-1;

    UnLock();
    return res;
}

static int FlashEmu_IoDread(PspIoDrvFileArg *arg, SceIoDirent *dir)
{
    int argv[2];

    argv[0] = (int)arg;
    argv[1] = (int)dir;

    return sceKernelExtendKernelStack(0x4000, (void *)dread_main, (void *)argv);
}

static int FlashEmu_IoDclose(PspIoDrvFileArg *arg)
{
    return sceKernelExtendKernelStack(0x4000, (void *)dclose_main, (void *)arg);
}

static int dopen_main(int *argv)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
    const char *dirname = (const char *)argv[1];

    Lock();
    BuildPath(dirname);

    int dfd = sceIoDopen(path);
    if (dfd < 0)
    {
        UnLock();
        return dfd;
    }

    arg->arg = (void *)dfd;

    UnLock();
    return 0;
}

static int FlashEmu_IoDopen(PspIoDrvFileArg *arg, const char *dirName)
{
    int argv[2];

    argv[0] = (int)arg;
    argv[1] = (int)dirName;

    return sceKernelExtendKernelStack(0x4000, (void *)dopen_main, (void *)argv);
}

static int rmdir_main(int *argv)
{
    const char *name = (const char *)argv[1];

    Lock();
    BuildPath(name);

    int res = sceIoRmdir(name);

    UnLock();
    return res;
}

static int FlashEmu_IoRmdir(PspIoDrvFileArg *arg, const char *name)
{
    int argv[2];

    argv[0] = (int)arg;
    argv[1] = (int)name;

    return sceKernelExtendKernelStack(0x4000, (void *)rmdir_main, (void *)argv);
}

static int mkdir_main(u32 *argv)
{
    const char *name = (const char *)argv[1];
    SceMode mode = (SceMode)argv[2];

    Lock();
    BuildPath(name);

    int res = sceIoMkdir(name, mode);

    UnLock();
    return res;
}


static int FlashEmu_IoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode)
{
    int argv[3];

    argv[0] = (int)arg;
    argv[1] = (int)name;
    argv[2] = (int)mode;

    return sceKernelExtendKernelStack(0x4000, (void *)mkdir_main, (void *)argv);
}

static int remove_main(u32 *argv)
{
    const char *file = (const char *)argv[1];

    Lock();
    BuildPath(file);

    int res = sceIoRemove(path);

    UnLock();
    return res;
}

static int FlashEmu_IoRemove(PspIoDrvFileArg *arg, const char *name)
{
    int argv[2];

    argv[0] = (int)arg;
    argv[1] = (int)name;

    return sceKernelExtendKernelStack(0x4000, (void *)remove_main, (void *)argv);
}

static int lseek_main(int *argv)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
    int ofs = (int)argv[1];
    int whence = argv[2];

    Lock();
    int res = sceIoLseek((SceUID)arg->arg, ofs, whence);
    UnLock();

    return res;
}

SceOff FlashEmu_IoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence)
{
    int argv[3];

    argv[0] = (int)arg;
    argv[1] = (int)ofs;
    argv[2] = (int)whence;

    return (SceOff)sceKernelExtendKernelStack(0x4000, (void *)lseek_main, (void *)argv);
}

static int write_main(int *argv)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
    const char *data = (const char *)argv[1];
    int len = argv[2];
    int res;

    Lock();
    if ((SceUID)arg->arg >= 0)
    {
        res = sceIoWrite((SceUID)arg->arg, data, len);
    }
    else
    {
        res = -1;
    }
    UnLock();

    return res;
}

static int FlashEmu_IoWrite(PspIoDrvFileArg *arg, const char *data, int len)
{
    int argv[3];

    argv[0] = (int)arg;
    argv[1] = (int)data;
    argv[2] = (int)len;

    return sceKernelExtendKernelStack(0x4000, (void *)write_main, (void *)argv);
}

static int read_main(int *argv)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
    char *data = (char *)argv[1];
    int len = argv[2];
    int res;

    Lock();
    if ((SceUID)arg->arg >= 0)
    {
        res = sceIoRead((SceUID)arg->arg, data, len);
    }
    else
    {
        res = -1;
    }

    UnLock();
    return res;
}

static int FlashEmu_IoRead(PspIoDrvFileArg *arg, char *data, int len)
{
    int argv[3];

    argv[0] = (int)arg;
    argv[1] = (int)data;
    argv[2] = (int)len;

    return sceKernelExtendKernelStack(0x4000, (void *)read_main, (void *)argv);
}

static int open_main(int *argv)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
    const char *file = (const char *)argv[1];
    int flags = argv[2];
    SceMode mode = (SceMode)argv[3];

    Lock();
    BuildPath(file);

    SceUID fd = sceIoOpen(path, flags, mode);
    arg->arg = (void *)fd;
    if (fd < 0)
    {
        UnLock();
        return -1;
    }

    UnLock();

    return 0;
}

static int FlashEmu_IoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode)
{
    int argv[4];

    argv[0] = (int)arg;
    argv[1] = (int)file;
    argv[2] = (int)flags;
    argv[3] = (int)mode;

    return sceKernelExtendKernelStack(0x4000, (void *)open_main, (void *)argv);
}

static int FlashEmu_IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
    int res;

    //Kprintf("Ioctl cmd 0x%08X\n", cmd);

    Lock();

    switch (cmd)
    {
    case 0x00008003: // FW1.50
    case 0x00208003: // FW2.00 load prx  "/vsh/module/opening_plugin.prx"

        res = 0;

        break;

    case 0x00208006: // load prx
        res = 0;
        break;

    case 0x00208007: // after start FW2.50
        res = 0;
        break;

    case 0x00208081: // FW2.00 load prx "/vsh/module/opening_plugin.prx"
        res = 0;
        break;

    case 0x00208082:      // FW2.80 "/vsh/module/opening_plugin.prx"
        res = 0x80010016; // opening_plugin.prx , mpeg_vsh,prx , impose_plugin.prx
        break;

    case 0x00005001: // vsh_module : system.dreg / system.ireg
        // Flush
        //res = sceKernelExtendKernelStack(0x4000, (void *)close_main, (void *)arg);
        res = 0;
        break;

    default:
        //Kprintf("Unknow ioctl 0x%08X\n", cmd);
        res = 0xffffffff;
    }

    UnLock();
    return res;
}

static int close_main(PspIoDrvFileArg *arg)
{
    int res = 0;

    if ((SceUID)arg->arg >= 0)
    {
        res = sceIoClose((SceUID)arg->arg);
    }
    else
    {
        res = 0;
    }
    arg->arg = (void *)-1;

    if (res < 0)
    {
        return res;
    }

    return 0;
}

static int FlashEmu_IoClose(PspIoDrvFileArg *arg)
{
    Lock();
    int res = sceKernelExtendKernelStack(0x4000, (void *)close_main, (void *)arg);
    UnLock();

    return res;
}

static int FlashEmu_IoInit()
{
    flashemu_sema = sceKernelCreateSema("FlashSema", 0, 1, 1, NULL);

    return 0;
}

void sceLfatfsWaitReady()
{
    if (flashemuThid >= 0)
        sceKernelWaitThreadEnd(flashemuThid, 0);

    return;
}

void WaitMS()
{
    SceUID dfd;
    if (msNotReady)
    {
        while (1)
        {
        	dfd = sceIoDopen(ARK_DC_PATH);
        	if (dfd >= 0)
        	{
        		sceIoDclose(dfd);
        		break;
        	}
        	sceKernelDelayThreadCB(20000);
        }

        msNotReady = 0;
    }

    return;
}

int SceLfatfsAssign()
{
    WaitMS();
    sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, 0, 0);
    sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, 0, 0);
    flashemuThid = -1;
    sceKernelExitDeleteThread(0);

    return 0;
}

int sceLfatfsStop()
{
    return 0;
}