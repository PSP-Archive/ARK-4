#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <module2.h>
#include <macros.h>
#include <string.h>
#include <systemctrl.h>
#include <ark.h>
#include <macros.h>

enum{ // internal id
    MODULE_NET_FTP,
    MODULE_AV_PNG,
    MODULE_AV_PLAYER,
    MODULE_VLF,
    MODULE_INTRAFONT_VLF,
    MODULE_INTRAFONT_GU,
    MODULE_UNARCHIVER,
    N_MODULES
};

typedef struct {
    int modid;
    char* prxname;
} CustomUtilityModule;

extern ARKConfig* ark_config;

static CustomUtilityModule custom_utility_modules[N_MODULES] = {
    {-1, PSPFTP_PRX},
    {-1, LIBPNG_PRX},
    {-1, PSPAV_PRX},
    {-1, VLF_PRX},
    {-1, "VLFFONT.PRX"},
    {-1, INTRAFONT_PRX},
    {-1, UNARCHIVE_PRX},
};

static int (*origUtilityLoadModule)(int);
static int (*origUtilityUnloadModule)(int);

static int loadstartCustomUtilityModule(int internal_id){
    CustomUtilityModule* module = &custom_utility_modules[internal_id];
    if (module->modid >= 0) return 0x80111102; // SCE_UTILITY_MODULE_ERROR_ALREADY_LOADED

    char modpath[ARK_PATH_SIZE];
    strcpy(modpath, ark_config->arkpath);
    strcat(modpath, module->prxname);

    module->modid = sceKernelLoadModule(modpath, 0, NULL);
    if (module->modid < 0) return module->modid;

    int res = sceKernelStartModule(module->modid, strlen(modpath)+1, (void*)modpath, NULL, NULL);
    if (res < 0){
        sceKernelUnloadModule(module->modid);
        module->modid = -1;
        return res;
    }
    return 0;
}

static int stopunloadCustomUtilityModule(int internal_id){
    CustomUtilityModule* module = &custom_utility_modules[internal_id];
    if (module->modid < 0) return 0x80111103; // SCE_UTILITY_MODULE_ERROR_NOT_LOADED

    int res = sceKernelStopModule(module->modid, 0, NULL, NULL, NULL);

    if (res >= 0){
        res = sceKernelUnloadModule(module->modid);
        if (res >= 0){
            module->modid = -1;
        }
    }
    
    return res;
}

static int extendedUtilityLoadModule(int module){
    int k1 = pspSdkSetK1(0);
    int res = -1;
    switch (module){
        case PSP_MODULE_NET_FTP:
            res = loadstartCustomUtilityModule(MODULE_NET_FTP);
            break;
        case PSP_MODULE_AV_PNG:
            res = loadstartCustomUtilityModule(MODULE_AV_PNG);
            break;
        case PSP_MODULE_AV_PLAYER:
            res = loadstartCustomUtilityModule(MODULE_AV_PLAYER);
            break;
        case PSP_MODULE_VLF:
            res = loadstartCustomUtilityModule(MODULE_VLF);
            break;
        case PSP_MODULE_INTRAFONT_VLF:
            res = loadstartCustomUtilityModule(MODULE_INTRAFONT_VLF);
            break;
        case PSP_MODULE_INTRAFONT_GU:
            res = loadstartCustomUtilityModule(MODULE_INTRAFONT_GU);
            break;
        case PSP_MODULE_UNARCHIVER:
            res = loadstartCustomUtilityModule(MODULE_UNARCHIVER);
            break;
        default:
            res = origUtilityLoadModule(module);
            break;
    }
    pspSdkSetK1(k1);
    return res;
}

static int extendedUtilityUnloadModule(int module){
    int k1 = pspSdkSetK1(0);
    int res = -1;
    switch (module){
        case PSP_MODULE_NET_FTP:
            res = stopunloadCustomUtilityModule(MODULE_NET_FTP);
            break;
        case PSP_MODULE_AV_PNG:
            res = stopunloadCustomUtilityModule(MODULE_AV_PNG);
            break;
        case PSP_MODULE_AV_PLAYER:
            res = stopunloadCustomUtilityModule(MODULE_AV_PLAYER);
            break;
        case PSP_MODULE_VLF:
            res = stopunloadCustomUtilityModule(MODULE_VLF);
            break;
        case PSP_MODULE_INTRAFONT_VLF:
            res = stopunloadCustomUtilityModule(MODULE_INTRAFONT_VLF);
            break;
        case PSP_MODULE_INTRAFONT_GU:
            res = stopunloadCustomUtilityModule(MODULE_INTRAFONT_GU);
            break;
        case PSP_MODULE_UNARCHIVER:
            res = stopunloadCustomUtilityModule(MODULE_UNARCHIVER);
            break;
        default:
            res = origUtilityUnloadModule(module);
            break;
    }
    pspSdkSetK1(k1);
    return res;
}

void extendUtilityModules(){
    u32 utilityLoadModule = sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0x2A2B3DE0);
    u32 utilityUnloadModule = sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xE49BFE92);

    HIJACK_FUNCTION(utilityLoadModule, extendedUtilityLoadModule, origUtilityLoadModule);
    HIJACK_FUNCTION(utilityUnloadModule, extendedUtilityUnloadModule, origUtilityUnloadModule);
}
