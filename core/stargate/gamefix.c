#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspdebug.h>
#include <pspinit.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <rebootconfig.h>
#include <ark.h>
#include <functions.h>

extern SEConfig* se_config;

static int (*utilityGetParam)(int, int*) = NULL;
static int getParamFixed_ULJM05221(int param, int* value){
    int res = utilityGetParam(param, value);
    if (param == PSP_SYSTEMPARAM_ID_INT_LANGUAGE && *value > 1){
        *value = 0;
    }
    return res;
}

static STMOD_HANDLER wwe_previous;
static void wweModuleOnStart(SceModule2 * mod)
{

    // Boot Complete Action not done yet
    if (strcmp(mod->modname, "mainPSP") == 0)
    {
        hookImportByNID(mod, "scePower", 0x34F9C463, 222); // scePowerGetPllClockFrequencyInt
        hookImportByNID(mod, "scePower", 0x843FBF43, 0);   // scePowerSetCpuClockFrequency
        hookImportByNID(mod, "scePower", 0xFDB5BFE9, 222); // scePowerGetCpuClockFrequencyInt
        hookImportByNID(mod, "scePower", 0xBD681969, 111); // scePowerGetBusClockFrequencyInt
    }

    // Call Previous Module Start Handler
    if(wwe_previous) wwe_previous(mod);
    
}

void applyFixesByModule(SceModule2* mod){

    // fix black screen in Tekken 6
    if (strcmp(mod->modname, "tekken") == 0) {
        hookImportByNID(mod, "scePower", 0x34F9C463, 222); // scePowerGetPllClockFrequencyInt
    }

    // remove "overclock" message in ATV PRO
    else if (strcmp(mod->modname, "ATVPRO") == 0){
        hookImportByNID(mod, "scePower", 0x843FBF43, 0);   // scePowerSetCpuClockFrequency
        hookImportByNID(mod, "scePower", 0xFDB5BFE9, 222); // scePowerGetCpuClockFrequencyInt
        hookImportByNID(mod, "scePower", 0xBD681969, 111); // scePowerGetBusClockFrequencyInt
    }

    // disable anti-CFW code
    else if (strcasecmp(mod->modname, "DJMAX") == 0) {
        // prevent detecting/deleting ISO folder
        hookImportByNID(mod, "IoFileMgrForUser", 0xE3EB004C, 0);

        // enable UMD reading speed
        void (*SetUmdDelay)(int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
        if (SetUmdDelay) SetUmdDelay(2);

        // disable Inferno Cache
        int (*CacheInit)(int, int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
        if (CacheInit) CacheInit(0, 0, 0);

        // prevent Inferno Cache from being re-enabled
        SEConfig* se_config = sctrlSEGetConfig(NULL);
        se_config->iso_cache = 0;
    }

    flushCache();
}

void applyFixesByGameId(){
    // Obtain game ID for other patches
    RebootConfigARK* reboot_config = sctrlHENGetRebootexConfig(NULL);
    char gameid[10]; memset(gameid, 0, sizeof(gameid));
    strncpy(gameid, reboot_config->game_id, 9);

    // Fix TwinBee Portable when not using English or Japanese language
    if (strcasecmp("ULJM05221", gameid) == 0){
        utilityGetParam = sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xA5DA2406);
        sctrlHENPatchSyscall(utilityGetParam, getParamFixed_ULJM05221);
    }

    // Patch Smakdown vs RAW 2011 anti-CFW check (CPU speed)
    else if (strcasecmp("ULES01472", gameid) == 0 || strcasecmp("ULUS10543", gameid) == 0){
        wwe_previous = sctrlHENSetStartModuleHandler(wweModuleOnStart);
    }

    // Patch Aces of War anti-CFW check (UMD speed)
    else if (strcasecmp("ULES00590", gameid) == 0 || strcasecmp("ULJM05075", gameid) == 0){
        void (*SetUmdDelay)(int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
        if (SetUmdDelay) SetUmdDelay(1);
    }
}