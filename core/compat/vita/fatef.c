#include "fatef.h"

extern PspIoDrvFuncs ms_funcs;

static int _sceIoEfOpen(u32* args)
{

    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *file = (char *)args[1];
	int flags = args[2];
	SceMode mode = (SceMode)args[3];

	char path[256];
	strcpy(path, "/__ef0__");
	strcat(path, file);
    
    // Forward Call
    return sceIoMsOpenHook(arg, path, flags, mode);
}

static int _sceIoEfRemove(u32 *args)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];

	char path[256];
	strcpy(path, "/__ef0__");
	strcat(path, file);
    
    // Forward Call
    return ms_funcs.IoRemove(arg, path);
}

static int _sceIoEfMkdir(u32* args)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceMode mode = (SceMode)args[2];

	char path[256];
	strcpy(path, "/__ef0__");
	strcat(path, file);
    
    // Forward Call
    return ms_funcs.IoMkdir(arg, path, mode);
}

static int _sceIoEfRmdir(u32* args)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];

	char path[256];
	strcpy(path, "/__ef0__");
	strcat(path, file);
    
    // Forward Call
    return ms_funcs.IoRmdir(arg, path);
}

static int _sceIoEfDopen(u32* args)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];

	char path[256];
	strcpy(path, "/__ef0__");
	strcat(path, file);
    
    // Forward Call
    return ms_funcs.IoDopen(arg, path);
}

static _sceIoEfGetStat(u32* args)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];

	char path[256];
	strcpy(path, "/__ef0__");
	strcat(path, file);
    
    // Forward Call
    return ms_funcs.IoGetstat(arg, path, stat);
}

static int _sceIoEfChStat(u32* args)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];
	int bits = (int)args[3];

	char path[256];
	strcpy(path, "/__ef0__");
	strcat(path, file);
    
    // Forward Call
    return ms_funcs.IoChstat(arg, path, stat, bits);
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
    return ms_funcs.IoRename(arg, oldname, newname);
}

static int _sceIoEfChdir(u32* args)
{
    PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *dir = (const char *)args[1];

	char path[256];
	strcpy(path, "/__ef0__");
	strcat(path, dir);

	// Forward Call
    return ms_funcs.IoChdir(arg, dir);
}

int sceIoEfOpenHook(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)flags;
	args[3] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfOpen, args);
}

int sceIoEfRemoveHook(PspIoDrvFileArg * arg, char * file)
{
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)file;

	return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfRemove, args);
}

int sceIoEfMkdirHook(PspIoDrvFileArg * arg, char * file, SceMode mode)
{
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfMkdir, args);
}

int sceIoEfRmdirHook(PspIoDrvFileArg * arg, char * file)
{
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)file;

	return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfRmdir, args);
}

int sceIoEfDopenHook(PspIoDrvFileArg * arg, char * file)
{
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)file;

	return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfDopen, args);
}

int sceIoEfGetStatHook(PspIoDrvFileArg * arg, char * file, SceIoStat* stat)
{
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;

	return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfGetStat, args);
}

int sceIoEfChStatHook(PspIoDrvFileArg * arg, char * file, SceIoStat* stat, int bits)
{
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;
	args[3] = (u32)bits;

	return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfChStat, args);
}

int sceIoEfRenameHook(PspIoDrvFileArg *arg, const char *oldname, const char *newname)
{
    u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)oldname;
	args[2] = (u32)newname;

    return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfRename, args);
}

int sceIoEfChdirHook(PspIoDrvFileArg *arg, const char *dir)
{
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dir;

	return sceKernelExtendKernelStack(0x4000, (void *)_sceIoEfChdir, args);
}