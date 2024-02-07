#include <pspkernel.h>
#include <kubridge.h>
#include <vector>
#include <sstream>
#include <fstream>
#include "gfx.h"
#include "debug.h"
#include "common.h"
#include "system_mgr.h"
#include "gamemgr.h"
#include "browser.h"
#include "settingsmenu.h"
#include "settings_entries.h"
#include "net_mgr.h"
#include "ftp_driver.h"
#include "exit_mgr.h"
#include "lang.h"

#include "ark_settings.h"
#include "ark_plugins.h"

PSP_MODULE_INFO("ARKMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH|PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(17*1024);

using namespace std;

#define MAX_ENTRIES 7
static SystemEntry* entries[MAX_ENTRIES];

extern "C" void my_malloc_init();

int main(int argc, char** argv){

    // make malloc/free threadsafe
    my_malloc_init();

    srand(time(NULL));

    int encoding = 5;
    sceIoDevctl("fatms0:", 0x02425856, &encoding, 4, NULL, 0);
    //sceIoDevctl("fatef0:", 0x02425856, &encoding, 4, NULL, 0);

    intraFontInit();
    ya2d_init();

    // setup UMD disc
    sceUmdReplacePermit();

    // Load data (theme, config, font, etc)
    common::loadData(argc, argv);

    // check to run last game
    Controller pad;
    int recovery = common::getArkConfig()->recovery;

    pad.update(1);
    if (pad.RT()){
        recovery = 1;
    }
    if (pad.LT()){
        common::stopLoadingThread();
        const char* last_game = common::getConf()->last_game;
        if (Eboot::isEboot(last_game)){
            Eboot* eboot = new Eboot(last_game);
            eboot->execute();
        }
        else if (Iso::isISO(last_game)){
            Iso* iso = new Iso(last_game);
            iso->execute();
        }
    }

    int n_entries = (recovery)?0:2;

    // Setup FTP
    if (common::getPspModel() != PSP_11000 && !recovery){
        entries[n_entries++] = new NetworkManager();
        // initialize FTP client driver for file browser
        Browser::ftp_driver = new FTPDriver();
    }

    // Add ARK settings manager
    loadSettings();
    SettingsTable cfwstab = { ark_conf_entries, ark_conf_max_entries };
    SettingsMenu* settings_menu = new SettingsMenu(&cfwstab, saveSettings, false, true, true);
    settings_menu->setCallbacks(NULL, loadSettings, cleanupSettings);
    settings_menu->setName("CFW Settings");
    settings_menu->setInfo("ARK Custom Firmware Settings");
    settings_menu->readConf();
    entries[n_entries++] = settings_menu;

    // Add ARK plugins manager
    SettingsMenu* plugins_menu = new SettingsMenu(&plugins_table, savePlugins, true, true, true);
    plugins_menu->setCallbacks(NULL, loadPlugins, cleanupPlugins);
    plugins_menu->setName("Plugins");
    plugins_menu->setInfo("Installed Plugins");
    plugins_menu->setIcon(IMAGE_PLUGINS);
    entries[n_entries++] = plugins_menu;

    // Setup settings
    int max_settings = MAX_SETTINGS_OPTIONS;
    if (common::getPspModel() != PSP_GO) max_settings -= 2;
    SettingsTable stab = { settings_entries, max_settings };
    entries[n_entries++] = new SettingsMenu(&stab, common::saveConf, false, true, true);

    if (recovery){
        entries[n_entries++] = Browser::getInstance();
    }

    // exit
    entries[n_entries++] = new ExitManager();

    if (!recovery){
        // Setup main App (Game or Browser)
        if (common::getConf()->main_menu == 0){
            entries[1] = Browser::getInstance();
            entries[0] = GameManager::getInstance();
        }
        else{
            entries[0] = Browser::getInstance();
            entries[1] = GameManager::getInstance();
        }
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

    sctrlKernelExitVSH(NULL);
    
    return 0;
}
