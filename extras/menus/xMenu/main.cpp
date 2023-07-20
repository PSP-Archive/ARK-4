#include <pspkernel.h>
#include <sstream>
#include "debug.h"
#include "common.h"
#include "menu.h"

PSP_MODULE_INFO("XMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

using namespace std;

std::string startup_txt = "starting menu";
std::string startup_txt_upper = "STARTING MENU";

int startup_thread(int argc, void* argp){
	int i;
	std::stringstream startup_holder;
	for(i=0;i<startup_txt.length();i++) {
		if((i%2)==0)
			startup_holder << startup_txt_upper[i];
		else 
			startup_holder << startup_txt[i];

		debugScreen(startup_holder.str().c_str(), 180, 130);
		sceKernelDelayThread(50000);
	}
	return 0;
}

/*
void doIoTest(){
	int fd, res;
	
	res = sceIoMkdir("ms0:/PSP/SAVEDATA/TEST01234", 0777);
	
	if (res < 0){
		startup_txt = startup_txt_upper = "mkdir error";
		return;
	}

	fd = sceIoOpen("ms0:/PSP/SAVEDATA/TEST01234/SCEVMC0.VMP", PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
	
	if (fd < 0){
		startup_txt = startup_txt_upper = "mkfile error";
		return;
	}

	sceIoWrite(fd, "test", sizeof("test"));

	sceIoClose(fd);

	res = sceIoRemove("ms0:/PSP/SAVEDATA/TEST01234/SCEVMC0.VMP");

	if (res < 0){
		startup_txt = startup_txt_upper = "rmfile error";
		return;
	}

	res = sceIoRmdir("ms0:/PSP/SAVEDATA/TEST01234");

	if (res < 0){
		startup_txt = startup_txt_upper = "rmdir error";
		return;
	}
}
*/

int main(int argc, char** argv){

    common::setArgs(argc, argv);

	//doIoTest();

	int thid = sceKernelCreateThread("xmenu bootup", (SceKernelThreadEntry)startup_thread, 10, 2048, PSP_THREAD_ATTR_VFPU, NULL);
	sceKernelStartThread(thid, 0, NULL);

    initGraphics();
    common::loadData();

	sceKernelWaitThreadEnd(thid, NULL);
	sceKernelDeleteThread(thid);

    Menu* menu = new Menu();
    menu->run();
    delete menu;
    
    common::deleteData();
    
    disableGraphics();
        
    sceKernelExitGame();

    return 0;
}
