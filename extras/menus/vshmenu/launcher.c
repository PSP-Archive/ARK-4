#include "launcher.h"

#include <stdlib.h>
#include <string.h>
#include <kubridge.h>
#include <pspthreadman.h>

#include "scepaf.h"
#include "vpl.h"


void exec_recovery_menu(vsh_Menu *vsh) {
    // try recovery app
    char menupath[ARK_PATH_SIZE];
    scePaf_strcpy(menupath, vsh->config.ark.arkpath);
    strcat(menupath, ARK_RECOVERY);

    SceIoStat stat; int res = sceIoGetstat(menupath, &stat);
    
    if (res >= 0){
        struct SceKernelLoadExecVSHParam param;
        scePaf_memset(&param, 0, sizeof(param));
        param.size = sizeof(param);
        param.args = scePaf_strlen(menupath) + 1;
        param.argp = menupath;
        param.key = "game";
        sctrlKernelLoadExecVSHWithApitype(0x141, menupath, &param);
    }

    memset(menupath, 0, sizeof(menupath));
    scePaf_strcpy(menupath, vsh->config.ark.arkpath);
    strcat(menupath, RECOVERY_PRX);
    res = sceIoGetstat(menupath, &stat);
    if(res < 0) {
    	memset(menupath, 0, sizeof(menupath));
    	scePaf_strcpy(menupath, RECOVERY_PRX_FLASH);
    }
    SceUID modid = kuKernelLoadModule(menupath, 0, NULL);
    if(modid >= 0) {
    	int res = sceKernelStartModule(modid, strlen(menupath) + 1, menupath, NULL, NULL);
    }

}
