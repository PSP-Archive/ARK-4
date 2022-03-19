#include <pspinit.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <string.h>
#include <stdio.h>
#include <module2.h>
#include <globals.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <functions.h>
#include <graphics.h>

// define all possible file replacements here
static struct{
    char* orig;
    char* new;
    unsigned char len;
} ioreplacements[] = {
    // Replace flash0 pops with custom one
    {
        .orig = "flash0:/vsh/module/libpspvmc.prx",
        .new = "PSPVMC.PRX",
        .len = 32,
    },
    {
        .orig = "flash0:/kd/pops_",
        .new = "POPS.PRX",
        .len = 15,
    },
    {
        .orig = "flash0:/kd/popsman.prx",
        .new = "POPSMAN.PRX",
        .len = 22,
    },
    {
        .orig = "flash0:/kd/npdrm.prx",
        .new = "NPDRM.PRX",
        .len = 20,
    }
};
int (*iojal)(const char *, u32, u32, u32, u32, u32) = NULL;
int patchio(const char *a0, u32 a1, u32 a2, u32 a3, u32 t0, u32 t1)
{

    int res;
    for (int i=0; i<NELEMS(ioreplacements); i++){
        if (strncmp(a0, ioreplacements[i].orig, ioreplacements[i].len) == 0){
            char path[ARK_PATH_SIZE];
            strcpy(path, ark_config->arkpath);
            strcat(path, ioreplacements[i].new);
            res = iojal(path, a1, a2, a3, t0, t1);
            if (res>=0) return res;
            break;
        }
    }
    res = iojal(a0, a1, a2, a3, t0, t1);
    
    return res;
}

u32 backup[2], ioctl;
int patchioctl(SceUID a0, u32 a1, void *a2, int a3, void *t0, int t1)
{
    _sw(backup[0], ioctl);
    _sw(backup[1], ioctl + 4);

    sceKernelDcacheWritebackInvalidateRange((void *)ioctl, 8);
    sceKernelIcacheInvalidateRange((void *)ioctl, 8);

    int ret = sceIoIoctl(a0, a1, a2, a3, t0, t1);

    _sw((((u32)&patchioctl >> 2) & 0x03FFFFFF) | 0x08000000, ioctl);
    _sw(0, ioctl + 4);

    sceKernelDcacheWritebackInvalidateRange((void *)ioctl, 8);
    sceKernelIcacheInvalidateRange((void *)ioctl, 8);

    if (ret < 0 && ((a1 & 0x208000) == 0x208000))
        return 0;

    return ret;
}

SceModule2* patchFileIO(){
    SceModule2 * mod = (SceModule2 *)sceKernelFindModuleByName("sceIOFileManager");
    u32 topaddr = mod->text_addr+mod->text_size;
    // find functions
    int patches = 2;
    for (u32 addr=mod->text_addr; addr<topaddr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x03641824){
            iojal = addr-4;
            patches--;
        }
        else if (data == 0x00005021){
            ioctl = addr-12;
            patches--;
        }
    }
    // redirect IO
    patches = 2;
    for (u32 addr=mod->text_addr; addr<topaddr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(iojal)){
            _sw(JAL(patchio), addr);
            patches--;
        }
    }
    
    // redirect ioctl to patched
    backup[0] = _lw(ioctl);
    backup[1] = _lw(ioctl + 4);
    _sw(JUMP(patchioctl), ioctl);
    _sw(0, ioctl+4);
    
    // Flush Cache
    flushCache();
    
    return mod;
}
