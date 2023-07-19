#include <pspkernel.h>
#include <sstream>
#include "debug.h"
#include "common.h"
#include "menu.h"

PSP_MODULE_INFO("XMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

using namespace std;

int main(int argc, char** argv){

    common::setArgs(argc, argv);

    initGraphics();

    common::loadData();

	std::string startup_txt = "starting menu";
	std::string startup_txt_upper = "STARTING MENU";

	std::stringstream startup_holder;

	int i;
	for(i=0;i<startup_txt.length();i++) {
			if((i%2)==0)
				startup_holder << startup_txt_upper[i];
			else 
				startup_holder << startup_txt[i];

		debugScreen(startup_holder.str().c_str(), 180, 130);
		sceKernelDelayThread(100000);
	}
		

    Menu* menu = new Menu();
    menu->run();
    delete menu;
    
    common::deleteData();
    
    disableGraphics();
        
    sceKernelExitGame();

    return 0;
}
