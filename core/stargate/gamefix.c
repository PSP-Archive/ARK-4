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

static STMOD_HANDLER game_previous;

static int (*utilityGetParam)(int, int*) = NULL;
static int getParamFixed_ULJM05221(int param, int* value){
    int res = utilityGetParam(param, value);
    if (param == PSP_SYSTEMPARAM_ID_INT_LANGUAGE && *value > 1){
        *value = 0;
    }
    return res;
}

static void wweModuleOnStart(SceModule2 * mod)
{
    // Boot Complete Action not done yet
    if (strcmp(mod->modname, "mainPSP") == 0)
    {
        sctrlHookImportByNID(mod, "scePower", 0x34F9C463, 222); // scePowerGetPllClockFrequencyInt
        sctrlHookImportByNID(mod, "scePower", 0x843FBF43, 0);   // scePowerSetCpuClockFrequency
        sctrlHookImportByNID(mod, "scePower", 0xFDB5BFE9, 222); // scePowerGetCpuClockFrequencyInt
        sctrlHookImportByNID(mod, "scePower", 0xBD681969, 111); // scePowerGetBusClockFrequencyInt
    }

    // Call Previous Module Start Handler
    if(game_previous) game_previous(mod);   
}


void applyFixesByModule(SceModule2* mod){

    // fix black screen in Tekken 6
    if (strcmp(mod->modname, "tekken") == 0) {
        sctrlHookImportByNID(mod, "scePower", 0x34F9C463, 222); // scePowerGetPllClockFrequencyInt
    }

    // remove "overclock" message in ATV PRO
    else if (strcmp(mod->modname, "ATVPRO") == 0){
        sctrlHookImportByNID(mod, "scePower", 0x843FBF43, 0);   // scePowerSetCpuClockFrequency
        sctrlHookImportByNID(mod, "scePower", 0xFDB5BFE9, 222); // scePowerGetCpuClockFrequencyInt
        sctrlHookImportByNID(mod, "scePower", 0xBD681969, 111); // scePowerGetBusClockFrequencyInt
    }

    // disable anti-CFW code
    else if (strcasecmp(mod->modname, "DJMAX") == 0) {
	hide_cfw_folder(mod);
        SEConfig* se_config = sctrlSEGetConfig(NULL);

        if (se_config->umdseek == 0 && se_config->umdspeed == 0){
            // enable UMD reading speed
            void (*SetUmdDelay)(int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
            if (SetUmdDelay) SetUmdDelay(2, 2);
            se_config->umdseek = 2;
            se_config->umdspeed = 2;
        }

        // disable Inferno Cache
        int (*CacheInit)(int, int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
        if (CacheInit) CacheInit(0, 0, 0);

        // disable memory stick cache
        msstorCacheInit(NULL);

        // prevent Inferno Cache and MS Cache from being re-enabled
        se_config->iso_cache = 0;
        se_config->msspeed = 0;
    }

    sctrlFlushCache();
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

    // Patch Smakdown vs RAW 2009, 2010, and 2011 anti-CFW check (CPU speed)
    else if (strcasecmp("ULES01472", gameid) == 0 || strcasecmp("ULUS10543", gameid) == 0 || 
		    strcasecmp("ULES01339", gameid) == 0 || strcasecmp("ULUS10452", gameid) == 0 ||
		    strcasecmp("ULKS46195", gameid) == 0 || strcasecmp("ULUS10384", gameid) == 0 ||
	   	    strcasecmp("ULES01165", gameid) == 0){
        game_previous = sctrlHENSetStartModuleHandler(wweModuleOnStart);
    }

    // Patch Aces of War anti-CFW check (UMD speed)
    else if (strcasecmp("ULES00590", gameid) == 0 || strcasecmp("ULJM05075", gameid) == 0){
        SEConfig* se_config = sctrlSEGetConfig(NULL);
        if (se_config->umdseek == 0 && se_config->umdspeed == 0){
            void (*SetUmdDelay)(int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
            if (SetUmdDelay) SetUmdDelay(1, 1);
            se_config->umdseek = 1;
            se_config->umdspeed = 1;
        }
    }
}
