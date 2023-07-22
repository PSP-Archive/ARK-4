#include <pspkernel.h>
#include <sstream>
#include "debug.h"
#include "common.h"
#include "menu.h"

PSP_MODULE_INFO("XMENU", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

using namespace std;

string startup_txt = "Loading ";

static uint8_t dots = 0;

static volatile bool loading = true;

int startup_thread(int argc, void* argp){
	int i;
	stringstream startup_runner;
	for(i=0;i<startup_txt.length();i++) {
	//void printText(float x, float y, const char *text, u32 color = WHITE_COLOR);
		startup_runner << startup_txt[i];	
	//	debugScreen(startup_runner.str().c_str(), 180, 130);
	//	sceKernelDelayThread(15000);
	}


	while (loading){
		debugScreen(startup_runner.str().c_str(), 180, 130);
		if(dots>3) {
			startup_runner.str(startup_txt);
			dots=0;
		}
		else {
			startup_runner.str(startup_txt + string(dots, '.'));
		}
		dots++;
		sceKernelDelayThread(200000);
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

	// start loading screen thread
	loading = true;
	int thid = sceKernelCreateThread("xmenu bootup", (SceKernelThreadEntry)startup_thread, 10, 2048, PSP_THREAD_ATTR_VFPU, NULL);
	sceKernelStartThread(thid, 0, NULL);

	// load data
    initGraphics();
    common::loadData();

	// initialize menu, scanning eboots
    Menu* menu = new Menu();

	// finish loading screen thread
	loading = false;
	sceKernelWaitThreadEnd(thid, NULL);
	sceKernelDeleteThread(thid);

	// run menu
    menu->run();

	// cleanup
    delete menu;
    common::deleteData();
    disableGraphics();

	// exit
    sceKernelExitGame();
    return 0;
}
