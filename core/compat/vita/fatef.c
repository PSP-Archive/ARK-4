#include "fatef.h"

extern PspIoDrvFuncs ms_funcs;

static int _sceIoEfHandler(u32* args)
{

    int (*IoFunc)(u32, u32, u32, u32) = args[0];
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[1];
    char *file = (char *)args[2];

    char path[256];
    strcpy(path, "/__ef0__");
    strcat(path, file);
    
    // Forward Call
    return IoFunc(arg, path, args[3], args[4]);
}

static int _sceIoEfRename(u32* args){
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
    const char *oldname = (const char *)args[1];
    const char *newname = (const char *)args[2];

    char oldpath[256];
    strcpy(oldpath, "/__ef0__");
    strcat(oldpath, oldname);

    char newpath[256];
    strcpy(newpath, "/__ef0__");
    strcat(newpath, newname);

    // Forward Call
    return ms_funcs.IoRename(arg, oldpath, newpath);
}

int sceIoEfOpenHook(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
    extern u32 sceIoMsOpenHook(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
    u32 args[5];
    args[0] = sceIoMsOpenHook;
    args[1] = (u32)arg;
    args[2] = (u32)file;
    args[3] = (u32)flags;
    args[4] = (u32)mode;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfHandler, args);
}

int sceIoEfRemoveHook(PspIoDrvFileArg * arg, char * file)
{
    u32 args[3];
    args[0] = ms_funcs.IoRemove;
    args[1] = (u32)arg;
    args[2] = (u32)file;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfHandler, args);
}

int sceIoEfMkdirHook(PspIoDrvFileArg * arg, char * file, SceMode mode)
{
    u32 args[4];
    args[0] = ms_funcs.IoMkdir;
    args[1] = (u32)arg;
    args[2] = (u32)file;
    args[3] = (u32)mode;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfHandler, args);
}

int sceIoEfRmdirHook(PspIoDrvFileArg * arg, char * file)
{
    u32 args[3];
    args[0] = ms_funcs.IoRmdir;
    args[1] = (u32)arg;
    args[2] = (u32)file;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfHandler, args);
}

int sceIoEfDopenHook(PspIoDrvFileArg * arg, char * file)
{
    u32 args[3];
    args[0] = ms_funcs.IoDopen;
    args[1] = (u32)arg;
    args[2] = (u32)file;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfHandler, args);
}

int sceIoEfGetStatHook(PspIoDrvFileArg * arg, char * file, SceIoStat* stat)
{
    u32 args[4];
    args[0] = ms_funcs.IoGetstat;
    args[1] = (u32)arg;
    args[2] = (u32)file;
    args[3] = (u32)stat;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfHandler, args);
}

int sceIoEfChStatHook(PspIoDrvFileArg * arg, char * file, SceIoStat* stat, int bits)
{
    u32 args[5];
    args[0] = ms_funcs.IoChstat;
    args[1] = (u32)arg;
    args[2] = (u32)file;
    args[3] = (u32)stat;
    args[4] = (u32)bits;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfHandler, args);
}

int sceIoEfChdirHook(PspIoDrvFileArg *arg, const char *dir)
{
    u32 args[3];
    args[0] = ms_funcs.IoChdir;
    args[1] = (u32)arg;
    args[2] = (u32)dir;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfHandler, args);
}

int sceIoEfRenameHook(PspIoDrvFileArg *arg, const char *oldname, const char *newname)
{
    u32 args[3];
    args[0] = (u32)arg;
    args[1] = (u32)oldname;
    args[2] = (u32)newname;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfRename, args);
}