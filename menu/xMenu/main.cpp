#include <pspkernel.h>
#include "xmenu/debug.h"
#include "xmenu/common.h"
#include "xmenu/menu.h"

PSP_MODULE_INFO("XMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

using namespace std;

int main(int argc, char** argv){

	common::setArgs(argc, argv);

	intraFontInit();
	initGraphics();

	common::loadData();

	debugScreen("starting menu");
	Menu* menu = new Menu();
	menu->run();
	delete menu;
	
	common::deleteData();
	
	intraFontShutdown();
	disableGraphics();
		
	sceKernelExitGame();
}
