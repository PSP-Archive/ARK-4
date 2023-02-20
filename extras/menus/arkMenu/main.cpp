#include <pspkernel.h>
#include <kubridge.h>
#include <fstream>
#include "gfx.h"
#include "debug.h"
#include "common.h"
#include "system_mgr.h"
#include "gamemgr.h"
#include "browser.h"
#include "settingsmenu.h"
#include "settings_entries.h"
#include "ftp_mgr.h"
#include "ftp_driver.h"
#include "exit_mgr.h"

PSP_MODULE_INFO("ARKMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(17*1024);

using namespace std;

#define MAX_ENTRIES 5
static SystemEntry* entries[MAX_ENTRIES];

int main(int argc, char** argv){

    intraFontInit();
    ya2d_init();

    // setup UMD disc
    sceUmdReplacePermit();

    // Load data (theme, config, font, etc)
    common::loadData(argc, argv);

    // initialize FTP client driver for file browser
    Browser::ftp_driver = new FTPDriver();

    int n_entries = 2;

    // Setup FTP App
    if (common::getPspModel() != PSP_11000){
        entries[n_entries++] = new FTPManager();
    }
    // Setup settings and exit
    SettingsTable stab = { settings_entries, MAX_SETTINGS_OPTIONS };
    entries[n_entries++] = new SettingsMenu(&stab, common::saveConf);
    entries[n_entries++] = new ExitManager();

    // Setup main App (Game or Browser)
    if (common::getConf()->main_menu == 0){
        entries[1] = new Browser();
        entries[0] = new GameManager();
        GameManager::updateGameList(NULL);
    }
    else{
        entries[0] = new Browser();
        entries[1] = new GameManager();
    }
    
    // Initialize Menu
    SystemMgr::initMenu(entries, n_entries);

    // Handle control to Menu
    SystemMgr::startMenu();

    // Cleanup data and exit
    SystemMgr::endMenu();
    common::deleteData();
    intraFontShutdown();
    ya2d_shutdown();

    sceKernelExitGame();
    
    return 0;
}
