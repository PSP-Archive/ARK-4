#include <pspkernel.h>
#include "gfx.h"
#include "debug.h"
#include "common.h"
#include "system_mgr.h"
#include "gamemgr.h"
#include "browser.h"
#include "vshmenu.h"
#include "vshmenu_entries.h"

PSP_MODULE_INFO("ARKMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(15*1024);

using namespace std;

#define MAX_ENTRIES 3
static SystemEntry* entries[MAX_ENTRIES];

int main(int argc, char** argv){

    intraFontInit();
    ya2d_init();

    try{

        common::loadData(argc, argv);

        entries[2] = new VSHMenu(vsh_entries, MAX_VSH_OPTIONS, common::saveConf);
        entries[1] = new Browser();
        entries[0] = new GameManager();
        
        SystemMgr::initMenu(entries, MAX_ENTRIES);
        
        SystemMgr::startMenu();
        SystemMgr::endMenu();

        common::deleteData();
    
    }
    catch (const std::exception& e){
        FILE* fp = fopen("EXCEPTION.TXT", "w");
        string msg = e.what();
        fwrite(msg.c_str(), 1, msg.size(), fp);
        fclose(fp);
    }
    
    intraFontShutdown();
    
    ya2d_shutdown();

    sceKernelExitGame();
    
    return 0;
}
