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
#include <globals.h>
#include <functions.h>


int (*utilityGetParam)(int, int*) = NULL;
int getParamFixed_ULJM05221(int param, int* value){
    int res = utilityGetParam(param, value);
    if (param == PSP_SYSTEMPARAM_ID_INT_LANGUAGE && *value > 1){
        *value = 0;
    }
    return res;
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
        hookImportByNID(mod, "IoFileMgrForUser", 0xE3EB004C, 0);
    }

    flushCache();
}

void applyFixesByGameId(){
    // Obtain game ID for other patches
    char gameid[10];
    memset(gameid, 0, sizeof(gameid));
    getGameId(gameid);

    // Fix TwinBee Portable when not using English or Japanese language
    if (strcasecmp("ULJM05221", gameid) == 0){
        utilityGetParam = sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xA5DA2406);
        sctrlHENPatchSyscall(utilityGetParam, getParamFixed_ULJM05221);
    }
}