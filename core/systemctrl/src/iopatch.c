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

// define all possible file replacements here
static struct{
    char* orig;
    char* new;
    unsigned char len;
} ioreplacements[] = {
    // Replace flash0 pops with custom one (internal popsloader)
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
};
int (*iojal)(const char *, u32, u32, u32, u32, u32) = NULL;
int patchio(const char *a0, u32 a1, u32 a2, u32 a3, u32 t0, u32 t1)
{
    for (int i=0; i<NELEMS(ioreplacements); i++){
        if (strncmp(a0, ioreplacements[i].orig, ioreplacements[i].len) == 0){
            char path[ARK_PATH_SIZE];
            strcpy(path, ark_config->arkpath);
            strcat(path, ioreplacements[i].new);
            int res = iojal(path, a1, a2, a3, t0, t1);
            if (res>=0) return res;
            break;
        }
    }
    return iojal(a0, a1, a2, a3, t0, t1);
}

SceModule2* patchFileIO(){
    SceModule2 * mod = (SceModule2 *)sceKernelFindModuleByName("sceIOFileManager");
    u32 topaddr = mod->text_addr+mod->text_size;
    int patches = 2;
    for (u32 addr=mod->text_addr; addr<topaddr; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x03641824){
            iojal = addr-4;
            break;
        }
    }
    patches = 2;
    for (u32 addr=mod->text_addr; addr<topaddr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(iojal)){
            _sw(JAL(patchio), addr);
            patches--;
        }
    }
    return mod;
}
