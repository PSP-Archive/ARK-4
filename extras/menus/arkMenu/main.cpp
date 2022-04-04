#include <pspkernel.h>
//#include <kubridge.h>
//#include <fstream>
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

/*
int kfunc(){
    std::ofstream output("RAMFS.TXT");
    output << "References found:" << endl;
    
    for (u32 a=KERNEL_BASE; a<0x88300000; a+=4){
        u32 d = _lw(a)&0xFFFF0000;
        if (d >= FLASH_SONY && d<=FLASH_SONY+FLASH_SIZE){
            SceModule2* (*FindModuleByAddress)(u32) = (SceModule2* (*)(u32))sctrlHENFindFunction("sceLoaderCore", "LoadCoreForKernel", 0xBC99C625);
            SceModule2* mod = FindModuleByAddress(a);
            output << "Found ref at " << (void*)a << " with data " << (void*)d;
            if (mod) output << " and module " << mod->modname << " at offset " << a-mod->text_addr;
            output << endl;
        }
    }
    
    output.close();
    return 0;
}
*/

int main(int argc, char** argv){

    
    //struct KernelCallArg args; memset(&args, 0, sizeof(args));
    //kuKernelCall((void*)((u32)kfunc|0x80000000), &args);

    intraFontInit();
    ya2d_init();

    // setup UMD disc
    sceUmdReplacePermit();

    try{

        common::loadData(argc, argv);

        Browser::ftp_driver = new FTPDriver();

        // System Entries
        entries[4] = new ExitManager();
        entries[3] = new SettingsMenu(settings_entries, MAX_SETTINGS_OPTIONS, common::saveConf);
        entries[2] = new FTPManager();
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
