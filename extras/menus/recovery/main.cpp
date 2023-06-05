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
#include "settingsmenu.h"
#include "../../arkMenu/include/settings_entries.h"
#include "exit_mgr.h"
#include "game_mgr.h"

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

    common::loadData(argc, argv);

    // Add ARK settings manager
    loadSettings();
    SettingsTable stab = { ark_conf_entries, ark_conf_max_entries };
    SettingsMenu* settings_menu = new SettingsMenu(&stab, saveSettings, false, true, true);
    settings_menu->setName("CFW Settings");
    settings_menu->setInfo("ARK Custom Firmware Settings");
    settings_menu->readConf();
    entries[0] = settings_menu;

    // Add ARK plugins manager
    loadPlugins();
    SettingsMenu* plugins_menu = new SettingsMenu(&plugins_table, savePlugins, true, true, true);
    plugins_menu->setName("Plugins");
    plugins_menu->setInfo("Installed Plugins");
    plugins_menu->setIcon(IMAGE_PLUGINS);
    entries[1] = plugins_menu;

    // Add browser
    entries[2] = new Browser();

	// Settings
    SettingsTable stab_recovery = { settings_entries, MAX_SETTINGS_OPTIONS };
    SettingsMenu* recovery_settings_menu = new SettingsMenu(&stab_recovery, common::saveConf, false, true, true);
	recovery_settings_menu->setName("Menu Settings");
	recovery_settings_menu->setInfo("Launcher/Recovery Settings");
	entries[3] = recovery_settings_menu;


    // Add exit game
    entries[4] = new ExitManager();

    SystemMgr::initMenu(entries, MAX_ENTRIES);
    
    SystemMgr::startMenu();
    SystemMgr::endMenu();

    common::deleteData();
    
    intraFontShutdown();
    
    ya2d_shutdown();

    sceKernelExitGame();
    
    return 0;
}
