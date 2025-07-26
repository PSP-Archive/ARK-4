#include <vitasdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "promote.h"
#include "pbp.h"
#include "install.h"
#include "ui.h"

static char* NeededDirectories[] = {
    "ux0:pspemu", 
    "ux0:pspemu/PSP",
    "ux0:pspemu/PSP/SAVEDATA",
    "ux0:pspemu/temp",
    "ux0:pspemu/temp/game",
    "ux0:pspemu/temp/game/PSP",
    "ux0:pspemu/temp/game/PSP/GAME",
    "ux0:pspemu/temp/game/PSP/GAME/" TITLE_ID,
    "ux0:pspemu/temp/game/PSP/LICENSE"
};

static char* NeededDirectoriesARKX[] = {
    "ux0:pspemu", 
    "ux0:pspemu/PSP",
    "ux0:pspemu/temp",
    "ux0:pspemu/temp/game",
    "ux0:pspemu/temp/game/PSP",
    "ux0:pspemu/temp/game/PSP/GAME",
    "ux0:pspemu/temp/game/PSP/GAME/" ARK_X,
    "ux0:pspemu/temp/game/PSP/LICENSE"
};

int checkTaiConfig() {
    char c = 0;
    int fd = sceIoOpen("ur0:tai/config.txt", SCE_O_RDONLY, 0777);
    sceIoLseek(fd, -1, SCE_SEEK_END);
    sceIoRead(fd, &c, 1);
    sceIoClose(fd);
    return (c == '\n');
}

int installAnalogPlugin() {
    updateUi("Checking for ARK Right Analog Plugin ...");
    int pluginCheck = sceIoOpen("ur0:tai/arkrightanalog.suprx", SCE_O_RDONLY, 0777);
    if(pluginCheck < 0) {
        updateUi("ARK Right Analog Plugin not found adding to config ...");
        CopyFileAndUpdateUi("app0:psp/arkrightanalog.suprx", "ur0:tai/arkrightanalog.suprx");
        int hasNewLine = checkTaiConfig();
        int addPlugin = sceIoOpen("ur0:tai/config.txt", SCE_O_CREAT | SCE_O_WRONLY | SCE_O_APPEND, 0777);
        static char pluginLine[] = "# Add second analog support to ARK\n*NPUZ01234\nur0:tai/arkrightanalog.suprx";
        if (!hasNewLine) sceIoWrite(addPlugin, "\n", 1);
        sceIoWrite(addPlugin, pluginLine, sizeof(pluginLine)-1);
        sceIoClose(addPlugin);
        return 1;
    }
    else {
        sceIoClose(pluginCheck);
        updateUi("ARK Right Analog Plugin found updating plugin and base game only ...");
        CopyFileAndUpdateUi("app0:psp/arkrightanalog.suprx", "ur0:tai/arkrightanalog.suprx");
        return 0;
    }
}

int installPS1Plugin() {
    updateUi("Checking for ARK-X PS1 Plugin ...");
    int pluginCheck = sceIoOpen("ur0:tai/ps1cfw_enabler.suprx", SCE_O_RDONLY, 0777);    
    if(pluginCheck < 0) {
        updateUi("ARK-X PS1 Plugin not found adding to config ...");
        CopyFileAndUpdateUi("app0:psx/ps1cfw_enabler.suprx", "ur0:tai/ps1cfw_enabler.suprx");
        int hasNewLine = checkTaiConfig();
        int addPlugin = sceIoOpen("ur0:tai/config.txt", SCE_O_CREAT | SCE_O_WRONLY | SCE_O_APPEND, 0777);
        static char pluginLine[] = "# ARK-X\n*SCPS10084\nur0:tai/ps1cfw_enabler.suprx";
        if (!hasNewLine) sceIoWrite(addPlugin, "\n", 1);
        sceIoWrite(addPlugin, pluginLine, sizeof(pluginLine)-1);
        sceIoClose(addPlugin);
        return 1;
    }
    else {
        sceIoClose(pluginCheck);
        updateUi("ARK-X PS1 Plugin found updating plugin and base game only ...");
        CopyFileAndUpdateUi("app0:psx/ps1cfw_enabler.suprx", "ur0:tai/ps1cfw_enabler.suprx");
        CopyTree("app0:psx/GAME", "ux0:/pspemu/PSP/GAME");
        return 0;
    }
}

size_t GetTotalNeededDirectories(int _ARK_X) {
    if(_ARK_X)
        return (sizeof(NeededDirectoriesARKX) / sizeof(char*));
    else
        return (sizeof(NeededDirectories) / sizeof(char*));
}

void createPspEmuDirectories(int _ARK_X) {
    if(_ARK_X) {
        for(size_t i = 0; i < GetTotalNeededDirectories(_ARK_X); i++){
            CreateDirAndUpdateUi(NeededDirectoriesARKX[i]);
        }
    }
    else {
        for(size_t i = 0; i < GetTotalNeededDirectories(0); i++){
            CreateDirAndUpdateUi(NeededDirectories[i]);
        }
    }
}

void genEbootSignature(char* ebootPath, char *gameID) {
    char ebootSigFilePath[MAX_PATH];
    char ebootSig[0x200];    
    unsigned char pbpHash[0x20];

    int swVer = 0;

    memset(ebootSig, 0x00, sizeof(ebootSig));
    memset(pbpHash, 0x00, sizeof(pbpHash));

    if(gameID != NULL)
        snprintf(ebootSigFilePath, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/__sce_ebootpbp", gameID);
    else
        snprintf(ebootSigFilePath, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/__sce_ebootpbp", TITLE_ID);

    
    updateUi("Calculating EBOOT.PBP Sha256 ...");
    HashPbp(ebootPath, pbpHash);

    updateUi("Generating EBOOT.PBP Signature ...");
    int res = _vshNpDrmEbootSigGenPsp(ebootPath, pbpHash, ebootSig, &swVer);
    if(res >= 0) {
        WriteFile(ebootSigFilePath, ebootSig, sizeof(ebootSig));
    }
}

void placePspGameData(char *gameID) {
    char ebootFile[MAX_PATH] = {0};
    char pbootFile[MAX_PATH] = {0};
    char rifFile[MAX_PATH] = {0};

    // get path to EBOOT.PBP and PBOOT.PBP
    if(gameID != NULL) {
        snprintf(rifFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/LICENSE/%s.rif", CONTENT_ID_ARK);
        snprintf(ebootFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/EBOOT.PBP", gameID);
        CopyFileAndUpdateUi("app0:psx/EBOOT.PBP", ebootFile);
        CopyFileAndUpdateUi("app0:rif/psx.rif", rifFile);
    } else {
        snprintf(rifFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/LICENSE/%s.rif", CONTENT_ID);
        snprintf(ebootFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/EBOOT.PBP", TITLE_ID);
        snprintf(pbootFile, MAX_PATH, "ux0:pspemu/temp/game/PSP/GAME/%s/PBOOT.PBP", TITLE_ID);
        CopyFileAndUpdateUi("app0:psp/EBOOT.PBP", ebootFile);
        CopyFileAndUpdateUi("app0:psp/PBOOT.PBP", pbootFile);
        CopyFileAndUpdateUi("app0:rif/game.rif", rifFile);
    }

    genEbootSignature(ebootFile, gameID);
}

void createBubble(char *gameID) {
    updateUi("Promoting ...");
    if(gameID != NULL)
        promoteCma("ux0:pspemu/temp/game", gameID, SCE_PKG_TYPE_PSP);
    else
        promoteCma("ux0:pspemu/temp/game", TITLE_ID, SCE_PKG_TYPE_PSP);
}

void copySaveFiles() {
    sceIoMkdir("ux0:/pspemu/PSP/SAVEDATA/ARK_01234", 0006);
    CopyTree("app0:save/ARK_01234", "ux0:/pspemu/PSP/SAVEDATA/ARK_01234");
}

void installARK4Only() {
    createPspEmuDirectories(0);
    placePspGameData(NULL);
    createBubble(NULL);
    copySaveFiles();
}

void installARKXOnly() {
    createPspEmuDirectories(1);
    placePspGameData("SCPS10084");
    createBubble("SCPS10084");
}

void doInstall() {
 installARK4Only();        // 
    installARKXOnly();        // 
    installAnalogPlugin();    // 
    installPS1Plugin();       // 
    taiReloadConfig();        // 
}

void taiReloadConfig(void) {
    updateUi("Reloading tai config...");
    sceKernelDelayThread(1000000); // 1 second for visibility
	}