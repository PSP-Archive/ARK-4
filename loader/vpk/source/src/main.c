#include <vitasdk.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>

#include "install.h"
#include "ui.h"

void waitCross(){
	SceCtrlData pad;
	while(1){
		memset(&pad, 0x00, sizeof(SceCtrlData));
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if(pad.buttons == SCE_CTRL_CROSS)
			break;
	}
}

int main(int argc, const char *argv[]) {
	uiInit();	
	
	displayMsg("Install?", "Press X to begin installation ...");
	waitCross();
	
	doInstall();
	
	displayMsg("Install Complete!", "Press X to close this application ...");
	waitCross();
	
	return 0;
}