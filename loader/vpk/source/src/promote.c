#include <vitasdk.h>
#include <string.h>
#include <psp2/sysmodule.h>
#include <psp2/promoterutil.h>

static int loadScePaf() {
    static uint32_t argp[] = { 0x180000, -1, -1, 1, -1, -1 };

    const SceSysmoduleOpt opt = { 0 }; // struttura corretta richiesta dalla funzione

    return sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(argp), argp, &opt);
}

static int unloadScePaf() {
    const SceSysmoduleOpt opt = { 0 }; // struttura corretta
    return sceSysmoduleUnloadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, 0, NULL, &opt);
}

int promoteCma(const char *path, const char *titleid, int type) {
    int res;

    ScePromoterUtilityImportParams promoteArgs;
    memset(&promoteArgs, 0x00, sizeof(ScePromoterUtilityImportParams));
    strncpy(promoteArgs.path, path, sizeof(promoteArgs.path) - 1);
    strncpy(promoteArgs.titleid, titleid, sizeof(promoteArgs.titleid) - 1);
    promoteArgs.type = type;
    promoteArgs.attribute = 0x1;

    res = loadScePaf();
    if (res < 0) return res;

    res = sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    if (res < 0) return res;

    res = scePromoterUtilityInit();
    if (res < 0) return res;

    res = scePromoterUtilityPromoteImport(&promoteArgs);
    if (res < 0) return res;

    res = scePromoterUtilityExit();
    if (res < 0) return res;

    res = sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
    if (res < 0) return res;

    res = unloadScePaf();
    if (res < 0) return res;

    return res;
}
