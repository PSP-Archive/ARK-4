#include <string.h>

#include <pspkernel.h>
#include <pspiofilemgr_kernel.h>
#include <pspsysevent.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <psputilsforkernel.h>

#include <ark.h>
#include <module2.h>

#include "flashemu.h"
#include "cptbl.h"

#define PSP_INIT_KEYCONFIG_APP 0x400
#define PSP_BOOT_FLASH3 0x80
#define SCE_ERROR_ERRNO_NOT_SUPPORTED 0x80010086

SceUID flashemuThid;
int msNotReady = 1;
int installed = 0;

extern PspSysEventHandler sysEventHandler;

typedef struct FlashEmuPrivate
{
    int virtual;
    union
    {
        struct
        {
            u8 *buffer;
            int offset;
            u32 size;
        } virtualinfo;
        void *ms_private;
    } uni;
} FlashEmuPrivate;

unsigned int *(* parsealias)(char *) = NULL;

SceIoDeviceEntry *getDeviceEntry(char *alias)
{
    if (!parsealias)
    {
         SceModule2 *mod = sceKernelFindModuleByName("sceIOFileManager");
         if (!mod) return NULL;
         parsealias = (void *)(mod->text_addr + 0x35D0);
    }

    unsigned int *u = parsealias(alias);
    if (!u)
    {
        return NULL;
    }

    return (SceIoDeviceEntry *) u[1];
}

SceIoDeviceEntry *getMSEntry()
{
    SceIoDeviceEntry *ms0 = 0;
    while (1)
    {
        ms0 = getDeviceEntry("ms0:");

        if (ms0)
            break;
        sceKernelDelayThread(20000);
    }
    return ms0;
}

static char *tm_path = ARK_DC_PATH;

static void BuildPath(char *buf, const char *file)
{
    //Copy DC path skipping "ms0:"
    strcpy(buf, &tm_path[4]);
    strcat(buf, file);
}

static int FlashEmu_IoExit(SceIoDeviceEntry* de)
{
    sceKernelDeleteHeap((SceUID)de->d_private);

    return 0;
}

static int FlashEmu_IoMount(SceIoIob *iob)
{
    return 0;
}

static int FlashEmu_IoUmount(SceIoIob *iob)
{
    return 0;
}

static int FlashEmu_IoDevctl(SceIoIob *iob, const char *devname, unsigned int cmd, void *indata, SceSize inlen, void *outdata, SceSize outlen)
{
    int res = 0;

    switch (cmd)
    {
    case 0x5802:
        res = 0;
        break;
    case 0xb803:
        res = 0;
        break;
    case 0x208813:
        res = 0;
        if(sceKernelGetModel() == 0 || iob->i_unit != 3)
            res = 0x80010016;

    default:
        //Kprintf("Unknown devctl command 0x%08X\n", res);
        while (1);
    }

    return res;
}

static int FlashEmu_IoCancel(SceIoIob *iob)
{
    return SCE_ERROR_ERRNO_NOT_SUPPORTED;
}

static int DummyReturnZero()
{
    return 0;
}

static int DummyReturnNotSupported()
{
    return SCE_ERROR_ERRNO_NOT_SUPPORTED;
}

static int FlashEmu_IoChdir(SceIoIob *iob, const char *dir)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    char ms_path[260];
    BuildPath(ms_path, dir);
    
    int ret = ms0->d_dp->dt_func->df_chdir(&fake_iob, ms_path);
    
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoRename(SceIoIob *iob, const char *oldname, const char *newname)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    char ms_oldname[260];
    BuildPath(ms_oldname, oldname);
    
    char ms_newname[260];
    BuildPath(ms_newname, newname);
    
    int ret = ms0->d_dp->dt_func->df_rename(&fake_iob, ms_oldname, ms_newname);
    
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoChstat(SceIoIob *iob, const char *file, SceIoStat *stat, int bits)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    char ms_path[260];
    BuildPath(ms_path, file);
    
    int ret = ms0->d_dp->dt_func->df_chstat(&fake_iob, ms_path, stat, bits);
    
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoGetstat(SceIoIob *iob, const char *file, SceIoStat *stat)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    char ms_path[260];
    BuildPath(ms_path, file);
    
    int ret = ms0->d_dp->dt_func->df_getstat(&fake_iob, ms_path, stat);	
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoDread(SceIoIob *iob, SceIoDirent *dir)
{
    if ((int)dir->d_private == 0xffffffff)
        dir->d_private = 0;

    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;

    int ret = ms0->d_dp->dt_func->df_dread(&fake_iob, dir);
    
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoDclose(SceIoIob *iob)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    int ret = ms0->d_dp->dt_func->df_dclose(&fake_iob);
    
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoDopen(SceIoIob *iob, const char *dir)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    char ms_path[260];
    BuildPath(ms_path, dir);
    
    int ret = ms0->d_dp->dt_func->df_dopen(&fake_iob, ms_path);	
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoRmdir(SceIoIob *iob, const char *dir)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    char ms_path[260];
    BuildPath(ms_path, dir);
    
    int ret = ms0->d_dp->dt_func->df_rmdir(&fake_iob, ms_path);
    
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoMkdir(SceIoIob *iob, const char *name, SceMode mode)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    char ms_path[260];
    BuildPath(ms_path, name);
    
    int ret = ms0->d_dp->dt_func->df_mkdir(&fake_iob, ms_path, mode);
    
    iob->i_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoRemove(SceIoIob *iob, const char *name)
{
    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = iob->i_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    char ms_path[260];
    BuildPath(ms_path, name);
    
    int ret = ms0->d_dp->dt_func->df_remove(&fake_iob, ms_path);
    
    iob->i_private = fake_iob.i_private;

    return ret;
}

SceOff FlashEmu_IoLseek(SceIoIob *iob, SceOff ofs, int whence)
{
   FlashEmuPrivate *fep = iob->i_private;
    if (fep->virtual)
    {
        switch(whence)
        {
            case PSP_SEEK_SET: fep->uni.virtualinfo.offset = ofs; break;
            case PSP_SEEK_CUR: fep->uni.virtualinfo.offset += ofs; break;
            case PSP_SEEK_END: fep->uni.virtualinfo.offset = fep->uni.virtualinfo.size - ofs; break;
        }
        return fep->uni.virtualinfo.offset;
    }

    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = fep->uni.ms_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    int ret = ms0->d_dp->dt_func->df_lseek(&fake_iob, ofs, whence);
    
    fep->uni.ms_private = fake_iob.i_private;

    return ret;
}

static SceSSize FlashEmu_IoWrite(SceIoIob *iob, void *data, SceSize len)
{
    FlashEmuPrivate *fep = iob->i_private;
    if (fep->virtual)
    {
        return 0x80010086;
    }

    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = fep->uni.ms_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    int ret = 0;
    
    if (data && len)
        ret = ms0->d_dp->dt_func->df_write(&fake_iob, data, len);
    
    fep->uni.ms_private = fake_iob.i_private;

    return ret;
}

static SceSSize FlashEmu_IoRead(SceIoIob *iob, void *data, SceSize len)
{
    FlashEmuPrivate *fep = iob->i_private;
    if (fep->virtual)
    {
        memcpy(data, &fep->uni.virtualinfo.buffer[fep->uni.virtualinfo.offset], len);
		fep->uni.virtualinfo.offset += len;
		return len;
    }

    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = fep->uni.ms_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    int ret = ms0->d_dp->dt_func->df_read(&fake_iob, data, len);
    
    fep->uni.ms_private = fake_iob.i_private;

    return ret;
}

static int FlashEmu_IoOpen(SceIoIob *iob, char *file, int flags, SceMode mode)
{
    if (strcmp("/codepage/cptbl.dat", file) == 0)
    {
        FlashEmuPrivate *fep = sceKernelAllocHeapMemory((SceUID)iob->i_de->d_private, sizeof(FlashEmuPrivate));		
        fep->virtual = 1;
        fep->uni.virtualinfo.buffer = (u8 *)&cptbl;
        fep->uni.virtualinfo.offset = 0;
        fep->uni.virtualinfo.size = size_cptbl;
        iob->i_private = fep;

        return 0;
    }
    
    SceIoDeviceEntry *ms0 = getMSEntry();

    if (!ms0)
    {
        return -1;
    }

    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = 0;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;

    char ms_path[260];
    BuildPath(ms_path, file);

    int ret = ms0->d_dp->dt_func->df_open(&fake_iob, ms_path, flags, mode);
    if (ret < 0)
    {
        return ret;
    }
    
    FlashEmuPrivate *fep = sceKernelAllocHeapMemory((SceUID)iob->i_de->d_private, sizeof(FlashEmuPrivate));	
    fep->virtual = 0;
    fep->uni.ms_private = fake_iob.i_private;
        
    iob->i_private = fep;

    return ret;
}

static int FlashEmu_IoClose(SceIoIob *iob)
{
    FlashEmuPrivate *fep = iob->i_private;
    if (fep->virtual)
    {
        sceKernelFreeHeapMemory((SceUID)iob->i_de->d_private, fep);
        return 0;
    }

    SceIoDeviceEntry *ms0 = getMSEntry();
    
    SceIoIob fake_iob;
    fake_iob.i_flgs = iob->i_flgs;
    fake_iob.i_unit = 0;
    fake_iob.i_de = ms0;
    fake_iob.d_type = iob->d_type;
    fake_iob.i_private = fep->uni.ms_private;
    fake_iob.i_cwd = iob->i_cwd;
    fake_iob.i_fpos = iob->i_fpos;
    fake_iob.i_thread = iob->i_thread;
    fake_iob.dummy = iob->dummy;
    
    int ret = ms0->d_dp->dt_func->df_close(&fake_iob);
    
    iob->i_private = 0;

    sceKernelFreeHeapMemory((SceUID)iob->i_de->d_private, fep);

    return ret;
}

static int FlashEmu_IoIoctl(SceIoIob *iob, unsigned int cmd, void *indata, SceSize inlen, void *outdata, SceSize outlen)
{
    int res;

    //Kprintf("Ioctl cmd 0x%08X\n", cmd);

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

    return res;
}

static int FlashEmu_IoInit(SceIoDeviceEntry* de)
{
    de->d_private = (void *)sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, 0x4000, 1, "");

    return 0;
}

int sceLFatFsDevkitVersion()
{
    return 0x6060110;
}

int sceLFatFs_driver_F1FBA85F()
{
    return 0;
}

int sceLFatFs_driver_BED8D616()
{
    return 0;
}

int sceLFatFs_driver_E63DDEB5;

void sceLfatfsWaitReady()
{
    if (flashemuThid >= 0)
        sceKernelWaitThreadEnd(flashemuThid, 0);

    return;
}

int SceLfatfsAssign()
{
    getMSEntry();
    sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, 0, 0);
    sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, 0, 0);

    int appType = sceKernelInitKeyConfig();

    if (appType == PSP_INIT_KEYCONFIG_VSH || appType == PSP_INIT_KEYCONFIG_GAME || appType == PSP_INIT_KEYCONFIG_APP) {
        sceIoAssign("flash2:","lflash0:0,2","flashfat2:", IOASSIGN_RDWR, 0, 0);
    }

    if (sceKernelGetModel() != 0)
    {
        if (appType == PSP_INIT_KEYCONFIG_VSH ||
            ((appType == PSP_INIT_KEYCONFIG_GAME || appType == PSP_INIT_KEYCONFIG_APP) && sceKernelBootFrom() == PSP_BOOT_FLASH3)) {
            sceIoAssign("flash3:","lflash0:0,3","flashfat3:", IOASSIGN_RDWR, 0, 0);
        }
    }

    flashemuThid = -1;
    sceKernelExitDeleteThread(0);

    return 0;
}

int sceLfatfsStop()
{
    return 0;
}

static SceIoDeviceFunction lflashFuncs = {
    (void*)DummyReturnZero,    	    //IoInit
    (void*)DummyReturnZero,    	    //IoExit
    (void*)DummyReturnZero,    	    //IoOpen
    (void*)DummyReturnZero,    	    //IoClose
    (void*)DummyReturnZero,    	    //IoRead
    (void*)DummyReturnZero,    	    //IoWrite
    (void*)DummyReturnZero,    	    //IoLseek
    (void*)DummyReturnZero,    	    //IoIoctl
    (void*)DummyReturnNotSupported, //IoRemove
    (void*)DummyReturnNotSupported, //IoMkdir
    (void*)DummyReturnNotSupported, //IoRmdir
    (void*)DummyReturnNotSupported, //IoDopen
    (void*)DummyReturnNotSupported, //IoDclose
    (void*)DummyReturnNotSupported, //IoDread
    (void*)DummyReturnNotSupported, //IoGetstat
    (void*)DummyReturnNotSupported, //IoChstat
    (void*)DummyReturnNotSupported, //IoRename
    (void*)DummyReturnNotSupported, //IoChdir
    (void*)DummyReturnNotSupported, //IoMount
    (void*)DummyReturnNotSupported, //IoUmount
    (void*)DummyReturnZero,    	    //IoDevctl
    (void*)DummyReturnNotSupported  //IoUnk21
};

static SceIoDeviceFunction flashFatFuncs = {
    (void*)FlashEmu_IoInit,
    (void*)FlashEmu_IoExit,
    (void*)FlashEmu_IoOpen,
    (void*)FlashEmu_IoClose,
    (void*)FlashEmu_IoRead,
    (void*)FlashEmu_IoWrite,
    (void*)FlashEmu_IoLseek,
    (void*)FlashEmu_IoIoctl,
    (void*)FlashEmu_IoRemove,
    (void*)FlashEmu_IoMkdir,
    (void*)FlashEmu_IoRmdir,
    (void*)FlashEmu_IoDopen,
    (void*)FlashEmu_IoDclose,
    (void*)FlashEmu_IoDread,
    (void*)FlashEmu_IoGetstat,
    (void*)FlashEmu_IoChstat,
    (void*)FlashEmu_IoRename,
    (void*)FlashEmu_IoChdir,
    (void*)FlashEmu_IoMount,
    (void*)FlashEmu_IoUmount,
    (void*)FlashEmu_IoDevctl,
    (void*)FlashEmu_IoCancel
};

static PspIoDrv lflashDrv = {"lflash", 0x4, 0x200, 0, (PspIoDrvFuncs *)&lflashFuncs};
static PspIoDrv flashFatDrv = {"flashfat", 0x1E0010, 1, "FAT over Flash", (PspIoDrvFuncs *)&flashFatFuncs};

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
