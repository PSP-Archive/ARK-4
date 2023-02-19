#include <pspkernel.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "gfx.h"
#include "debug.h"
#include "common.h"
#include "system_mgr.h"
#include "gamemgr.h"
#include "browser.h"
#include "settingsmenu.h"
#include "ark_settings.h"
#include "ark_plugins.h"
#include "exit_mgr.h"
#include "game_mgr.h"

PSP_MODULE_INFO("ARKMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(17*1024);

using namespace std;

#define MAX_ENTRIES 4
static SystemEntry* entries[MAX_ENTRIES];

int main(int argc, char** argv){

    intraFontInit();
    ya2d_init();

    common::loadData(argc, argv);

    // Add ARK settings manager
    loadSettings();
    SettingsMenu* settings_menu = new SettingsMenu(ark_conf_entries, ark_conf_max_entries, saveSettings);
    settings_menu->setName("Settings");
    settings_menu->setInfo("ARK Settings");
    settings_menu->readConf();
    entries[0] = settings_menu;

    // Add ARK plugins manager
    loadPlugins();
    SettingsMenu* plugins_menu = new SettingsMenu(ark_plugin_entries, ark_plugins_count, savePlugins);
    plugins_menu->setName("Plugins");
    plugins_menu->setInfo("ARK Plugins");
    plugins_menu->setIcon(common::getImage(IMAGE_PLUGINS));
    entries[1] = plugins_menu;

    // Add browser
    entries[2] = new Browser();

    // Add exit game
    entries[3] = new ExitManager();

    SystemMgr::initMenu(entries, MAX_ENTRIES);
    
    SystemMgr::startMenu();
    SystemMgr::endMenu();

    common::deleteData();
    
    intraFontShutdown();
    
    ya2d_shutdown();

    sceKernelExitGame();
    
    return 0;
}
