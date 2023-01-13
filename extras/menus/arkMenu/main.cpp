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

    // Setup System Apps
    entries[4] = new ExitManager();
    entries[3] = new SettingsMenu(settings_entries, MAX_SETTINGS_OPTIONS, common::saveConf);
    entries[2] = new FTPManager();

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
    SystemMgr::initMenu(entries, MAX_ENTRIES);

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
